/*
 * netapi.c
 *
 * Copyright (C) 2013 loongsky development.
 *
 * Sam Chen <xuejian1354@163.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "netapi.h"
#include <nopoll.h>
#include <module/netlist.h>
#include <services/corecomm.h>
#include <protocol/protocol.h>
#include <protocol/request.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TRANS_HTTP_REQUEST
void curl_post_request(void *ptr);
#endif
#ifdef TRANS_WS_CONNECT
void *ws_read_func(void *p);
#endif
static void set_deudp_flag(uint8 flag);
static void set_detcp_flag(uint8 flag);
static void set_depost_flag(uint8 flag);

static uint8 deudp_flag = 1;
static uint8 detcp_flag = 1;
static uint8 depost_flag = 1;
static uint8 dews_flag = 1;
static uint8 deuart_flag = 1;
#ifdef DE_TRANS_UDP_STREAM_LOG
static struct sockaddr_in ulog_addr;
#endif

#if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG)
static frhandler_arg_t t_arg;
static int udpfd;
static struct sockaddr_in m_addr;
#endif
#ifdef TRANS_TCP_SERVER
static int s_tcpfd;
#endif
#ifdef TRANS_TCP_CLIENT
static int c_tcpfd;
static struct sockaddr_in m_server_addr;
#endif
#ifdef TRANS_WS_CONNECT
static noPollConn * ws_conn;
#endif

uint8 lwflag = 0;

frhandler_arg_t *get_frhandler_arg_alloc(int fd, struct sockaddr_in *addr, char *buf, int len)
{
	if(len > MAXSIZE)
	{
		return NULL;
	}

	frhandler_arg_t *arg = (frhandler_arg_t *)calloc(1, sizeof(frhandler_arg_t));
	arg->buf = (char *)calloc(1, len);

	arg->fd = fd;

	if(addr != NULL)
	{
		memcpy(&arg->addr, addr, sizeof(struct sockaddr_in));
	}
	
	if(buf != NULL)
	{
		memcpy(arg->buf, buf, len);
		arg->len = len;
	}
	else
	{
		free(arg->buf);
		arg->buf = NULL;
		arg->len = 0;
	}

	return arg;
}

void get_frhandler_arg_free(frhandler_arg_t *arg)
{
	if(arg != NULL)
	{
		free(arg->buf);
		free(arg);
	}
}

#ifdef COMM_TARGET
frhandler_arg_t *get_transtocol_frhandler_arg()
{
#if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG)
	t_arg.addr.sin_family = PF_INET;
	t_arg.addr.sin_port = htons(get_udp_port());
	t_arg.addr.sin_addr.s_addr = inet_addr(get_server_ip());

	return &t_arg;
#else
	return NULL;
#endif
}
#endif

#ifdef TRANS_TCP_SERVER
int get_stcp_fd()
{
	return s_tcpfd;
}

int socket_tcp_server_init(int port)
{
	struct sockaddr_in m_addr;
	
	if ((s_tcpfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("tcp socket fail");
		return -1;
	}
	
	m_addr.sin_family = PF_INET;
	m_addr.sin_port = htons(port);
	m_addr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("192.168.1.1");
	if (bind(s_tcpfd, (struct sockaddr *)&m_addr, sizeof(struct sockaddr)) < 0)
	{
		perror("bind tcp ip fail");
		return -1;
	}
	
	listen(s_tcpfd, 12);
	
#ifdef SELECT_SUPPORT
	select_set(s_tcpfd);
#endif
	set_trans_protocol(TOCOL_TCP);
	return 0;
}

int socket_tcp_server_accept(int fd)
{
	int rw;
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	
	rw = accept(fd, (struct sockaddr *)&client_addr, &len);
	
#ifdef DE_PRINT_TCP_PORT
	trans_data_show(DE_TCP_ACCEPT, &client_addr, "", 0);
#endif

#ifdef TRANS_TCP_CONN_LIST
	tcp_conn_t *m_list;
	
	m_list = (tcp_conn_t *)malloc(sizeof(tcp_conn_t));
	m_list->fd = rw;
	m_list->tclient = COMM_TCLIENT;
	m_list->client_addr = client_addr;
	m_list->next = NULL;

	if (addto_tcpconn_list(m_list) < 0)
	{
		free(m_list);
		close(rw);
		return 0;
	}
#endif

#ifdef SELECT_SUPPORT
	select_set(rw);
#endif

	return 0;
}

void socket_tcp_server_send(frhandler_arg_t *arg, char *data, int len)
{
	if(arg == NULL)
	{
		return;
	}

	send(arg->fd, data, len, 0);

#ifdef DE_PRINT_TCP_PORT
#ifdef TRANS_TCP_CONN_LIST
	tcp_conn_t *tconn = queryfrom_tcpconn_list(arg->fd);
	if(tconn != NULL)
	{
		arg->addr = tconn->client_addr;
	}
#endif

	trans_data_show(DE_TCP_SEND, &arg->addr, data, len);
#endif
}
#endif

#if defined(TRANS_TCP_SERVER) || (defined(COMM_TARGET) && defined(UART_COMMBY_SOCKET))
void socket_tcp_server_release(int fd)
{
	close(fd);
#ifdef SELECT_SUPPORT
	select_clr(fd);
#endif

#ifdef DE_PRINT_TCP_PORT
#ifdef TRANS_TCP_CONN_LIST
	tcp_conn_t *m_list = queryfrom_tcpconn_list(fd);
	if(m_list != NULL)
	{
		trans_data_show(DE_TCP_RELEASE, &m_list->client_addr, "", 0);
	}
	delfrom_tcpconn_list(fd);
#else
	DE_PRINTF(1, "TCP:release,fd=%d\n\n", fd);
#endif
#endif
}

int socket_tcp_server_recv(int fd)
{
	int nbytes;
	char buf[MAXSIZE];
	
	memset(buf, 0, sizeof(buf));
   	if ((nbytes = recv(fd, buf, sizeof(buf), 0)) <= 0)
   	{
      	socket_tcp_server_release(fd);
	}
	else
	{
#ifdef DE_PRINT_TCP_PORT
#ifdef TRANS_TCP_CONN_LIST
		tcp_conn_t *m_list = queryfrom_tcpconn_list(fd);
		if(m_list != NULL && m_list->tclient == COMM_TCLIENT)
		{
			trans_data_show(DE_TCP_RECV, &m_list->client_addr, buf, nbytes);
		}
#endif
#else
		DE_PRINTF(0, "data:%s\n", buf);
#endif

#if defined(COMM_TARGET) && defined(UART_COMMBY_SOCKET)
		if(m_list->tclient == RESER_TCLIENT)
		{
			serial_write(buf, nbytes);
			return 0;
		}
#endif

#ifdef TRANS_TCP_CONN_LIST
		frhandler_arg_t *frarg = 
			get_frhandler_arg_alloc(fd, &m_list->client_addr, buf, nbytes);
#ifdef THREAD_POOL_SUPPORT
		tpool_add_work(analysis_capps_frame, frarg, TPOOL_LOCK);
#else
		analysis_capps_frame(frarg);
#endif
#endif
	}
	return 0;
}
#endif

#ifdef TRANS_TCP_CLIENT
int get_ctcp_fd()
{
	return c_tcpfd;
}

int socket_tcp_client_connect(int port)
{
	if ((c_tcpfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("client tcp socket fail");
		return -1;
	}

	m_server_addr.sin_family = PF_INET;
	m_server_addr.sin_port = htons(port);
	m_server_addr.sin_addr.s_addr = inet_addr(get_server_ip());

	if(connect(c_tcpfd, (struct sockaddr *)&m_server_addr, sizeof(m_server_addr)) < 0)
	{
		perror("client tcp socket connect server fail");
		return -1;
	}
	DE_PRINTF(1, "new tcp client connection: fd=%d\n", c_tcpfd);

#ifdef SELECT_SUPPORT
	select_set(c_tcpfd);
#endif
	return 0;
}

int socket_tcp_client_recv()
{
	int nbytes;
	char buf[MAXSIZE];
	
	memset(buf, 0, sizeof(buf));
   	if ((nbytes = recv(c_tcpfd, buf, sizeof(buf), 0)) <= 0)
   	{
      	socket_tcp_client_close();
	}
	else
	{
#ifdef DE_PRINT_TCP_PORT
		trans_data_show(DE_TCP_RECV, &m_server_addr, buf, nbytes);
#endif
		frhandler_arg_t *frarg = 
			get_frhandler_arg_alloc(c_tcpfd, TOCOL_TCP, &m_server_addr, buf, nbytes);
#ifdef THREAD_POOL_SUPPORT
		tpool_add_work(analysis_capps_frame, frarg, TPOOL_LOCK);
#else
		analysis_capps_frame(frarg);
#endif
	}

	return 0;
}

void socket_tcp_client_send(char *data, int len)
{
	send(c_tcpfd, data, len, 0);

#ifdef DE_PRINT_TCP_PORT
	trans_data_show(DE_TCP_SEND, &m_server_addr, data, len);
#endif
}

void socket_tcp_client_close()
{
	close(c_tcpfd);
	DE_PRINTF(1, "close tcp client connection: fd=%d\n", c_tcpfd);
#ifdef SELECT_SUPPORT
	select_wtclr(c_tcpfd);
#endif
}
#endif

#if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG)
int get_udp_fd()
{
	return udpfd;
}

int socket_udp_service_init(int port)
{	
	if((udpfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("udp socket fail");
		return -1;
	}

	m_addr.sin_family = PF_INET;
	m_addr.sin_port = htons(port);
	m_addr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("192.168.1.1");

#ifdef DE_TRANS_UDP_STREAM_LOG
	ulog_addr.sin_family = PF_INET;
	ulog_addr.sin_port = htons(DE_UDP_PORT);
	ulog_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
#endif
	
	if (bind(udpfd, (struct sockaddr *)&m_addr, sizeof(struct sockaddr)) < 0)
	{
		perror("bind udp ip fail");
		return -1;
	}

#ifdef SELECT_SUPPORT
	select_set(udpfd);
#endif
	return 0;
}


int socket_udp_recvfrom()
{
	int nbytes;
	char buf[MAXSIZE];

	struct sockaddr_in client_addr;
	socklen_t socklen = sizeof(client_addr);

	memset(buf, 0, sizeof(buf));
	
	nbytes = recvfrom(udpfd, buf, sizeof(buf), 0, 
				(struct sockaddr *)&client_addr, &socklen);

#ifdef DE_PRINT_UDP_PORT
	trans_data_show(DE_UDP_RECV, &client_addr, buf, nbytes);
#endif

#if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG)
	frhandler_arg_t *frarg = 
		get_frhandler_arg_alloc(udpfd, &client_addr, buf, nbytes);
#ifdef THREAD_POOL_SUPPORT
	tpool_add_work(analysis_capps_frame, frarg, TPOOL_LOCK);
#else
	analysis_capps_frame(frarg);
#endif
#endif
	return 0;
}

void socket_udp_sendto_with_ipaddr(char *ipaddr, char *data, int len)
{
	int i,dlen;
	char saddr[16] = {0};
	int iport = 0;
	
	dlen = strlen(ipaddr);
	if(ipaddr!=NULL && dlen>0 && dlen<24)
	{
		for(i=0; i<dlen; i++)
		{
			if(*(ipaddr+i) == ':')
				break;
		}

		if(i!=0 && i!=dlen)
		{
			memcpy(saddr, ipaddr, i);
			iport = atoi(ipaddr+i+1);
		}
	}

	if(saddr[0]==0 || iport==0)
	{
		DE_PRINTF(1, "error ip address, ipaddr=%s\n", ipaddr);
		return;
	}
	
	struct sockaddr_in maddr;

	maddr.sin_family = PF_INET;
	maddr.sin_port = htons(iport);
	maddr.sin_addr.s_addr = inet_addr(saddr);

	socket_udp_sendto(&maddr, data, len);
}

void socket_udp_sendto(struct sockaddr_in *addr, char *data, int len)
{
#ifdef TRANS_UDP_SERVICE
	sendto(udpfd, data, len, 0, (struct sockaddr *)addr, sizeof(struct sockaddr));
#endif

#ifdef DE_PRINT_UDP_PORT
	trans_data_show(DE_UDP_SEND, addr, data, len);
	//PRINT_HEX(data, len);
#endif
}
#endif

void set_deudp_flag(uint8 flag)
{
	deudp_flag = flag;
}

void set_detcp_flag(uint8 flag)
{
	detcp_flag = flag;
}

void set_depost_flag(uint8 flag)
{
	depost_flag = flag;
}

void set_deuart_flag(uint8 flag)
{
	deuart_flag = flag;
}

int get_deuart_flag()
{
	return deuart_flag;
}

#ifdef DE_TRANS_UDP_STREAM_LOG
void delog_udp_sendto(char *data, int len)
{
#ifdef DAEMON_PROCESS_CREATE
	if(!get_daemon_cmdline())
	{
		char *pdata = (char *)calloc(1, len+1);
		memcpy(pdata, data, len);
		printf("%s", pdata);
		free(pdata);
		return;
	}
#endif

	sendto(udpfd, data, len, 0, 
		(struct sockaddr *)&ulog_addr, sizeof(struct sockaddr));
}
#endif

#ifdef TRANS_HTTP_REQUEST
void curl_http_request(curl_method_t cm, char *req, data_handler reback)
{
	char url[128] = {0};
	sprintf(url, "http://%s/devicedata", get_server_ip());

	switch(cm)
	{
	case CURL_GET:
		break;

	case CURL_POST:
	{
		curl_args_t *cargs = (curl_args_t *)calloc(1, sizeof(curl_args_t));
		cargs->cm = CURL_POST;
		cargs->url = url;
		cargs->req = req;
		cargs->curl_callback = reback;
		
#ifdef THREAD_POOL_SUPPORT
		tpool_add_work(curl_post_request, cargs, TPOOL_NONE);
#else
		curl_post_request(cargs);
#endif
	}
		break;
	}
}

void curl_post_request(void *ptr)
{
	curl_args_t *arg = (curl_args_t *)ptr;
	CURL *curl;  
    CURLcode res;
	FILE *fptr;

	if(arg == NULL)
	{
		DE_PRINTF(1, "%s()%d : curl does not get data\n", 
					__FUNCTION__, __LINE__); 
		return;
	}

	//struct curl_slist *http_header = NULL;
  
    curl = curl_easy_init();  
    if (!curl)  
    {  
        DE_PRINTF(1, "%s()%d : curl init failed\n", 
					__FUNCTION__, __LINE__); 
		return;
    }

	/*if ((fptr = fopen("/tmp/aaa.txt", "w")) == NULL) {   
        DE_PRINTF(1, "fopen file error: %s\n", "/tmp/aaa.txt");   
        return -1;
    }*/
  
    curl_easy_setopt(curl, CURLOPT_URL, arg->url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, arg->req);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, arg->curl_callback);
    //curl_easy_setopt(curl, CURLOPT_WRITEDATA, fptr);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    //curl_easy_setopt(curl, CURLOPT_HEADER, 1);
    //curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    //curl_easy_setopt(curl, CURLOPT_COOKIEFILE,"/tmp/curlpost.cookie");

#ifdef DE_PRINT_UDP_PORT
	trans_data_show(DE_POST_SEND,
		(struct sockaddr_in *)arg->url, arg->req, strlen(arg->req));
#else
	DE_PRINTF(0, "%s\nHTTP-Post: %s?%s\n\n",
		get_time_head(), arg->url, arg->req);
#endif
	
    res = curl_easy_perform(curl);  
  
    if (res != CURLE_OK)
    {
        switch(res)
        {
            case CURLE_UNSUPPORTED_PROTOCOL:
                DE_PRINTF(1, "%s()%d : Unsupport protocol\n",
					__FUNCTION__, __LINE__);
				break;
				
            case CURLE_COULDNT_CONNECT:
                DE_PRINTF(1, "%s()%d : Cannot connect host\n",
					__FUNCTION__, __LINE__);
				break;
				
            case CURLE_HTTP_RETURNED_ERROR:
                DE_PRINTF(1, "%s()%d : Return error\n",
					__FUNCTION__, __LINE__);
				break;
				
            case CURLE_READ_ERROR:
                DE_PRINTF(1, "%s()%d : Read file error\n",
					__FUNCTION__, __LINE__);
				break;
				
            default:
                DE_PRINTF(1, "%s()%d : Return %d\n",
					__FUNCTION__, __LINE__, res);
				break;
        }
    }
  
    curl_easy_cleanup(curl);
	free(arg);
}

size_t curl_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
#if 0
	xmlDocPtr doc;
	xmlNodePtr curNode;
	xmlChar *valData;
	
	doc = xmlReadMemory(buffer, size*nmemb, NULL, "utf8", XML_PARSE_RECOVER);
	if(NULL == doc)
	{
		DE_PRINTF(1, "Document not parsed successfully\n");
		goto curl_data_end;
	}

	curNode = xmlDocGetRootElement(doc);
	if (NULL == curNode)
	{ 
		DE_PRINTF(1, "empty document\n"); 
		goto curl_data_end;
    }

	if(xmlStrcmp(curNode->name, BAD_CAST "html")) 
    {
		DE_PRINTF(1, "document of the wrong type\n"); 
		goto curl_data_end;
	}
	
    curNode = curNode->xmlChildrenNode;
    while(curNode != NULL) 
    {
		if((!xmlStrcmp(curNode->name, (const xmlChar *)"body"))) 
		{
			curNode = curNode->xmlChildrenNode;
			while(curNode != NULL)
			{
				if((!xmlStrcmp(curNode->name, (const xmlChar *)"data"))) 
				{
					valData = xmlNodeGetContent(curNode);
					//DE_PRINTF(1, "name:%s\ncontent:%s\n", curNode->name, valData); 
					frhandler_arg_t *arg = (frhandler_arg_t *)calloc(1, sizeof(frhandler_arg_t));
					arg->transtocol = TOCOL_HTTP;
					arg->buf = valData;
					arg->len = strlen(valData);
					analysis_capps_frame(arg);
					xmlFree(valData); 
					break;
				}
				curNode = curNode->next; 
			}
			break;
		} 

		curNode = curNode->next; 
	}

curl_data_end:
	xmlFreeDoc(doc);

#else
	int i;
	int head = -1;
	int tail = -1;
	int len;

	char *data = (char *)buffer;
	len = strlen(data);

	for(i=0; i<len; i++)
	{
		if(*(data+i) == ' ')
		{
			continue;
		}
		else if(*(data+i) == '[')
		{
			head = i;
			break;
		}
	}

	for(i=len-1; i>=0; i--)
	{
		if(*(data+i) == ' ')
		{
			continue;
		}
		else if(*(data+i) == ']')
		{
			tail = i;
			break;
		}
	}

	if(head >= 0 && tail > 0 && tail > head + 1)
	{
		frhandler_arg_t *arg = (frhandler_arg_t *)calloc(1, sizeof(frhandler_arg_t));
		char *data_buf = (char *)calloc(1, tail-head);
		memcpy(data_buf, data+head+1, tail-head-1);
		arg->buf = data_buf;
		arg->len = tail - head - 1;

#ifdef DE_PRINT_UDP_PORT
		trans_data_show(DE_POST_RET, NULL, data, strlen(data));
#else
		DE_PRINTF(0, "%s\nHTTP-Returns: %s\n\n\n", get_time_head(), data);
#endif

		analysis_capps_frame(arg);
	}
	else
	{
		DE_PRINTF(1, "Errror format for returns\nreturn data: %s\n\n\n", data);
	}
#endif

	return nmemb*size;
}
#endif

#ifdef TRANS_WS_CONNECT
int ws_init()
{
	int i,dlen;
	char sip[16] = {0};
	char sport[8] = {0};
	int iport = 0;

	char url[128] = {0};
	sprintf(url, "ws://%s:%d", get_server_ip(), TRANS_WS_PORT);

	dlen = strlen(url);
	if(url!=NULL && dlen>12 && !strncmp(url, "ws://", 5))
	{
		for(i=5; i<dlen; i++)
		{
			if(*(url+i) == ':')
				break;
		}

		memcpy(sip, url+5, i-5);
		if(i < dlen-1)
		{
			iport = atoi(url+i+1);
		}

		if(iport)
		{
			sprintf(sport, "%d", iport);
		}
		else
		{
			sprintf(sport, "%d", 80);
		}
	}
	else
	{
		DE_PRINTF(1, "%s()%d : WebSocket url error\n",
					__FUNCTION__, __LINE__);
		return -1;
	}

	noPollCtx *ctx = nopoll_ctx_new();
	ws_conn = nopoll_conn_new(ctx, sip, sport, NULL, "/", NULL, NULL);
	if (!nopoll_conn_is_ok(ws_conn)) {
		DE_PRINTF(1, "%s()%d : WebSocket connect error\n",
					__FUNCTION__, __LINE__);
		return -1;
	}

	pthread_t wsRead;
	pthread_create(&wsRead, NULL, ws_read_func, NULL);

	return 0;
}

void ws_send(char *data, int len)
{
	if(nopoll_conn_send_text(ws_conn, data, len) != len)
	{
		DE_PRINTF(1, "%s()%d : WebSocket send error\n",
					__FUNCTION__, __LINE__);
	}
	else
	{
#ifdef DE_PRINT_UDP_PORT
		trans_data_show(DE_WS_SEND, NULL, data, len);
#else
		DE_PRINTF(0, "%s\nWS-Send: %s\n\n\n", get_time_head(), data);
#endif
	}
}

void *ws_read_func(void *p)
{
	char buffer[MAXSIZE];
	int bytes_read;

	while(1)
	{
		while (nopoll_true) {
			if (nopoll_conn_is_ready(ws_conn))
				break;
			nopoll_sleep(10000);
		}

		memset(buffer, 0, sizeof(buffer));
		bytes_read = nopoll_conn_read(ws_conn, buffer, sizeof(buffer), nopoll_true, 10);
		if(bytes_read > 0)
		{
			frhandler_arg_t *arg = (frhandler_arg_t *)calloc(1, sizeof(frhandler_arg_t));
			char *data_buf = (char *)calloc(1, bytes_read);
			memcpy(data_buf, buffer, bytes_read);
			arg->buf = data_buf;
			arg->len = bytes_read;

#ifdef DE_PRINT_UDP_PORT
			trans_data_show(DE_WS_RECV, NULL, data_buf, bytes_read);
#else
			DE_PRINTF(0, "%s\nWS-Recv: %s\n\n\n", get_time_head(), data_buf);
#endif
			analysis_capps_frame(arg);
		}
	}
}
#endif

void enable_datalog_atime()
{
	lwflag = 1;
}

void trans_data_show(de_print_t deprint, 
		struct sockaddr_in *addr, char *data, int len)
{
#ifdef DE_TRANS_UDP_STREAM_LOG
	if(deprint == DE_UDP_RECV
		&& addr->sin_port == htons(DE_UDP_PORT)
		&& addr->sin_addr.s_addr == inet_addr("127.0.0.1"))
	{
		if(len >= 7 && !memcmp(data, DEU_UDP_CMD, 5))
		{
			if(!memcmp(data+5, " 0", 2))
			{
				set_deudp_flag(0);
				DE_PRINTF(1, "\n%s disable\n\n", DEU_UDP_CMD);
			}
			else if(!memcmp(data+5, " 1", 2))
			{
				set_deudp_flag(1);
				DE_PRINTF(1, "\n%s enable\n\n", DEU_UDP_CMD);
			}
			else
			{
				DE_PRINTF(1, "\n%s set failed\n\n", DEU_UDP_CMD);
			}
		}
		else if(len >= 7 && !memcmp(data, DEU_TCP_CMD, 5))
		{
			if(!memcmp(data+5, " 0", 2))
			{
				set_detcp_flag(0);
				DE_PRINTF(1, "\n%s disable\n\n", DEU_TCP_CMD);
			}
			else if(!memcmp(data+5, " 1", 2))
			{
				set_detcp_flag(1);
				DE_PRINTF(1, "\n%s enable\n\n", DEU_TCP_CMD);
			}
			else
			{
				DE_PRINTF(1, "\n%s set failed\n\n", DEU_TCP_CMD);
			}
		}
		else if(len >= 8 && !memcmp(data, DEU_POST_CMD, 6))
		{
			if(!memcmp(data+6, " 0", 2))
			{
				set_depost_flag(0);
				DE_PRINTF(1, "\n%s disable\n\n", DEU_POST_CMD);
			}
			else if(!memcmp(data+6, " 1", 2))
			{
				set_depost_flag(1);
				DE_PRINTF(1, "\n%s enable\n\n", DEU_POST_CMD);
			}
			else
			{
				DE_PRINTF(1, "\n%s set failed\n\n", DEU_POST_CMD);
			}
		}
		else if(len >= 8 && !memcmp(data, DEU_UART_CMD, 6))
		{
			if(!memcmp(data+6, " 0", 2))
			{
				set_deuart_flag(0);
				DE_PRINTF(1, "\n%s disable set\n\n", DEU_UART_CMD);
			}
			else if(!memcmp(data+6, " 1", 2))
			{
				set_deuart_flag(1);
				DE_PRINTF(1, "\n%s enable\n\n", DEU_UART_CMD);
			}
			else
			{
				DE_PRINTF(1, "\n%s set failed\n\n", DEU_UART_CMD);
			}
		}
		else
		{
			DE_PRINTF(0, "\nUnrecognized cmd:%s", data);
			DE_PRINTF(0, "follow:\n");
			DE_PRINTF(0, "  %s [0|1]\n", DEU_UDP_CMD);
			DE_PRINTF(0, "  %s [0|1]\n", DEU_TCP_CMD);
			DE_PRINTF(0, "  %s [0|1]\n", DEU_POST_CMD);
			DE_PRINTF(0, "  %s [0|1]\n", DEU_UART_CMD);
		}

		return;
	}
#endif
	
	if( deprint == DE_UDP_SEND)
	{
		if(!deudp_flag)
		{
			return;
		}
		
		DE_PRINTF(lwflag, "%s\nUDP:send %d bytes, to ipaddr=%s:%u\n", 
					get_time_head(),
					len, inet_ntoa(addr->sin_addr), 
					ntohs(addr->sin_port));
	}
	else if(deprint == DE_UDP_RECV)
	{
		if(!deudp_flag)
		{
			return;
		}

#ifndef TRANS_UDP_SERVICE
		return;
#endif
		DE_PRINTF(lwflag, "%s\nUDP:receive %d bytes, from ipaddr=%s:%u\n", 
					get_time_head(),
					len, inet_ntoa(addr->sin_addr), 
					ntohs(addr->sin_port));
	}
	else if(deprint == DE_TCP_ACCEPT)
	{
		if(!detcp_flag)
		{
			return;
		}
		
		DE_PRINTF(1, "%s\nTCP:accept,ipaddr=%s:%u\n\n", 
			get_time_head(),
			inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
		return;
	}
	else if(deprint == DE_TCP_SEND)
	{
		if(!detcp_flag)
		{
			return;
		}
		
		DE_PRINTF(lwflag, "%s\nTCP:send %d bytes, to ipaddr=%s:%u\n", 
					get_time_head(),
					len, inet_ntoa(addr->sin_addr), 
					ntohs(addr->sin_port));
	}
	else if(deprint == DE_TCP_RECV)
	{
		if(!detcp_flag)
		{
			return;
		}
		
		DE_PRINTF(lwflag, "%s\nTCP:receive %d bytes, from ipaddr=%s:%u\n", 
					get_time_head(),
					len, inet_ntoa(addr->sin_addr), 
					ntohs(addr->sin_port));
	}
	else if(deprint == DE_TCP_RELEASE)
	{
		if(!detcp_flag)
		{
			return;
		}
		DE_PRINTF(1, "%s\nTCP:release,ipaddr=%s:%u\n\n", 
						get_time_head(),
						inet_ntoa(addr->sin_addr), 
						ntohs(addr->sin_port));
		return;
	}
	else if(deprint == DE_POST_SEND)
	{
		if(!depost_flag)
		{
			return;
		}
		DE_PRINTF(0, "%s\nHTTP-Post: %s?%s\n\n", get_time_head(), (char *)addr, data);
		return;
	}
	else if(deprint == DE_POST_RET)
	{
		if(!depost_flag)
		{
			return;
		}
		DE_PRINTF(0, "%s\nHTTP-Returns: %s\n\n\n", get_time_head(), data);
		return;
	}
	else if(deprint == DE_WS_SEND)
	{
		if(!dews_flag)
		{
			return;
		}
		DE_PRINTF(0, "%s\nWS-Send: %s\n\n\n", get_time_head(), data);
		return;
	}else if(deprint == DE_WS_RECV)
	{
		if(!dews_flag)
		{
			return;
		}
		DE_PRINTF(0, "%s\nWS-Recv: %s\n\n\n", get_time_head(), data);
		return;
	}
	else
	{
		return;
	}
				
	DE_PRINTF(lwflag, "data:%s\n\n", data);
	lwflag = 0;
}

#ifdef __cplusplus
}
#endif

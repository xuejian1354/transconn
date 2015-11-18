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
#include <module/netlist.h>
#include <module/balancer.h>
#include <protocol/protocol.h>
#include <sys/socket.h>
#include <arpa/inet.h>

typedef enum
{
	DE_UDP_SEND,
	DE_UDP_RECV,
	DE_TCP_ACCEPT,
	DE_TCP_SEND,
	DE_TCP_RECV,
	DE_TCP_RELEASE
}de_print_t;

#ifdef TRANS_HTTP_REQUEST
void curl_post_request(void *ptr);
#endif
static void set_deudp_flag(uint8 flag);
static void set_detcp_flag(uint8 flag);

static void trans_data_show(de_print_t deprint,
				struct sockaddr_in *addr, char *data, int len);

static uint8 deudp_flag = 1;
static uint8 detcp_flag = 1;
static uint8 deuart_flag = 1;
#ifdef DE_TRANS_UDP_STREAM_LOG
static struct sockaddr_in ulog_addr;
#endif

#if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG)
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

uint8 lwflag = 0;

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
}

void socket_tcp_server_accept(int fd)
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
	m_list->client_addr = client_addr;
	m_list->next = NULL;

	if (addto_tcpconn_list(m_list) < 0)
	{
		free(m_list);
		close(rw);
		return;
	}
#endif

#ifdef SELECT_SUPPORT
	select_set(rw);
#endif
}

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

void socket_tcp_server_recv(int fd)
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
		if(m_list != NULL)
		{
			trans_data_show(DE_TCP_RECV, &m_list->client_addr, buf, nbytes);
		}
#endif
#else
		DE_PRINTF(0, "data:%s\n", buf);
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
}

void socket_tcp_server_send(frhandler_arg_t *arg, char *data, int len)
{
	if(arg == NULL)
	{
		return;
	}
	
	send(arg->fd, data, len, 0);

#ifdef DE_PRINT_TCP_PORT
	trans_data_show(DE_TCP_SEND, &arg->addr, data, len);
#endif
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
}

void socket_tcp_client_recv()
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
			get_frhandler_arg_alloc(c_tcpfd, &m_server_addr, buf, nbytes);
#ifdef THREAD_POOL_SUPPORT
		tpool_add_work(analysis_capps_frame, frarg, TPOOL_LOCK);
#else
		analysis_capps_frame(frarg);
#endif
	}
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
}


void socket_udp_recvfrom()
{
	int nbytes;
	char buf[MAXSIZE];

	struct sockaddr_in client_addr;
	int socklen = sizeof(client_addr);

	memset(buf, 0, sizeof(buf));
	
	nbytes = recvfrom(udpfd, buf, sizeof(buf), 0, 
				(struct sockaddr *)&client_addr, &socklen);

#ifdef DE_PRINT_UDP_PORT
	trans_data_show(DE_UDP_RECV, &client_addr, buf, nbytes);
#endif

#if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG)
	frhandler_arg_t *frarg = get_frhandler_arg_alloc(udpfd, &client_addr, buf, nbytes);
#ifdef THREAD_POOL_SUPPORT
	tpool_add_work(analysis_capps_frame, frarg, TPOOL_LOCK);
#else
	analysis_capps_frame(frarg);
#endif
#endif
}

void socket_udp_sendto(char *addr, char *data, int len)
{
	int i,dlen;
	char saddr[16] = {0};
	int iport = 0;
	
	dlen = strlen(addr);
	if(addr!=NULL && dlen>0 && dlen<24)
	{
		for(i=0; i<dlen; i++)
		{
			if(*(addr+i) == ':')
				break;
		}

		if(i!=0 && i!=dlen)
		{
			memcpy(saddr, addr, i);
			iport = atoi(addr+i+1);
		}
	}

	if(saddr[0]==0 || iport==0)
	{
		DE_PRINTF(1, "error ip address, addr=%s\n", addr);
		return;
	}
	
	struct sockaddr_in maddr;

	maddr.sin_family = PF_INET;
	maddr.sin_port = htons(iport);
	maddr.sin_addr.s_addr = inet_addr(saddr);

#ifdef TRANS_UDP_SERVICE
	sendto(udpfd, data, len, 0, 
		(struct sockaddr *)&maddr, sizeof(struct sockaddr));
#endif

#ifdef DE_PRINT_UDP_PORT
	trans_data_show(DE_UDP_SEND, &maddr, data, len);
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
		char *pdata = calloc(1, len+1);
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
void curl_http_request(curl_method_t cm, 
		char *url, char *req, data_handler reback)
{
	switch(cm)
	{
	case CURL_GET:
		break;

	case CURL_POST:
	{
		curl_args_t *cargs = calloc(1, sizeof(curl_args_t));
		cargs->cm = CURL_POST;
		cargs->url = url;
		cargs->req = req;
		cargs->curl_callback = reback;
		
#ifdef THREAD_POOL_SUPPORT
		tpool_add_work(curl_post_request, cargs, TPOOL_LOCK);
#else
		curl_post_request(cargs);
		free(cargs);
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
		curl_easy_cleanup(curl);
		return;
    }
  
    curl_easy_cleanup(curl);

	free(arg);
}

size_t curl_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
	//FILE *fptr = (FILE*)userp;   
    //fwrite(buffer, size, nmemb, fptr);
	//DE_PRINTF(1, "%s\n", buffer);

	xmlDocPtr doc;
	xmlNodePtr curNode;
	xmlChar *szKey;
	
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
		if((!xmlStrcmp(curNode->name, (const xmlChar *)"head"))) 
		{
			curNode = curNode->xmlChildrenNode;
			while(curNode != NULL)
			{
				if((!xmlStrcmp(curNode->name, (const xmlChar *)"title"))) 
				{
					szKey = xmlNodeGetContent(curNode);
					DE_PRINTF(1, "name:%s\ncontent:%s\n", curNode->name, szKey); 
					xmlFree(szKey); 
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
	return nmemb*size;
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
		
		DE_PRINTF(lwflag, "UDP:send %d bytes, to ip=%s:%u\n", 
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
		DE_PRINTF(lwflag, "UDP:receive %d bytes, from ip=%s:%u\n", 
					len, inet_ntoa(addr->sin_addr), 
					ntohs(addr->sin_port));
	}
	else if(deprint == DE_TCP_ACCEPT)
	{
		if(!detcp_flag)
		{
			return;
		}
		
		DE_PRINTF(1, "TCP:accept,ip=%s:%u\n\n", 
			inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
	}
	else if(deprint == DE_TCP_SEND)
	{
		if(!detcp_flag)
		{
			return;
		}
		
		DE_PRINTF(lwflag, "TCP:send %d bytes, to ip=%s:%u\n", 
					len, inet_ntoa(addr->sin_addr), 
					ntohs(addr->sin_port));
	}
	else if(deprint == DE_TCP_RECV)
	{
		if(!detcp_flag)
		{
			return;
		}
		
		DE_PRINTF(lwflag, "TCP:receive %d bytes, from ip=%s:%u\n", 
					len, inet_ntoa(addr->sin_addr), 
					ntohs(addr->sin_port));
	}
	else if(deprint == DE_TCP_RELEASE)
	{
		if(!detcp_flag)
		{
			return;
		}
		DE_PRINTF(1, "TCP:release,ip=%s:%u\n\n", 
						inet_ntoa(addr->sin_addr), 
						ntohs(addr->sin_port));
		return;
	}
	else
	{
		return;
	}
				
	DE_PRINTF(lwflag, "data:%s\n", data);
	lwflag = 0;
}


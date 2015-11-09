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
#include <protocol/trframelysis.h>
#include <protocol/protocol.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#if(DE_PRINT_UDP_PORT!=0)
typedef enum
{
	DE_UDP_SEND,
	DE_UDP_RECV
}deudp_print_t;

static void set_udp_print_flag(uint8 flag);
static void udp_data_show(deudp_print_t deprint, uint8 flag,
		struct sockaddr_in *addr, char *data, int len);

static uint8 udp_print_flag = DE_PRINT_UDP_PORT;
#ifdef DE_TRANS_UDP_STREAM_LOG
static struct sockaddr_in ulog_addr;
#endif
#endif

#ifdef TRANS_UDP_SERVICE
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
	DE_PRINTF(1, "TCP:accept,ip=%s:%u\n\n", 
		inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
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
		DE_PRINTF(1, "TCP:release,ip=%s:%u\n\n", 
			inet_ntoa(m_list->client_addr.sin_addr), 
			ntohs(m_list->client_addr.sin_port));
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
#ifdef TRANS_TCP_CONN_LIST
		tcp_conn_t *m_list = queryfrom_tcpconn_list(fd);
		if(m_list != NULL)
		{
#ifdef DE_PRINT_TCP_PORT
			DE_PRINTF(0, "TCP:receive %d bytes, from ip=%s:%u\n", 
				nbytes, inet_ntoa(m_list->client_addr.sin_addr), 
				ntohs(m_list->client_addr.sin_port));
#endif
		}
#endif
		DE_PRINTF(0, "data:%s\n", buf);

#ifdef TRANS_TCP_CONN_LIST
		frhandler_arg_t *frarg = 
			get_frhandler_arg_alloc(fd, &m_list->client_addr, buf, nbytes);
#ifdef TIMER_SUPPORT
		tpool_add_work(analysis_capps_frame, frarg);
#else
		analysis_capps_frame(frarg, NULL);
#endif
#endif
	}
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

void socket_tcp_client_recv(int fd)
{
	int nbytes;
	char buf[MAXSIZE];
	
	memset(buf, 0, sizeof(buf));
   	if ((nbytes = recv(fd, buf, sizeof(buf), 0)) <= 0)
   	{
      	socket_tcp_client_close(fd);
	}
	else
	{
#ifdef DE_PRINT_TCP_PORT
		DE_PRINTF(0, "TCP:receive %d bytes, from ip=%s:%u\n", 
			nbytes, inet_ntoa(m_server_addr.sin_addr), 
			ntohs(m_server_addr.sin_port));

		DE_PRINTF(0, "data:%s\n", buf);
#endif
	}
}

void socket_tcp_client_close(int fd)
{
	close(fd);
	DE_PRINTF(1, "close tcp client connection: fd=%d\n", fd);
#ifdef SELECT_SUPPORT
	select_wtclr(fd);
#endif
}
#endif

#ifdef TRANS_UDP_SERVICE
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

#if(DE_PRINT_UDP_PORT!=0)
	udp_data_show(DE_UDP_RECV, udp_print_flag, &client_addr, buf, nbytes);
#endif

	frhandler_arg_t *frarg = get_frhandler_arg_alloc(udpfd, &client_addr, buf, nbytes);
#ifdef TIMER_SUPPORT
	tpool_add_work(analysis_capps_frame, frarg);
#else
	analysis_capps_frame(frarg, NULL);
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
	
	sendto(udpfd, data, len, 0, 
		(struct sockaddr *)&maddr, sizeof(struct sockaddr));

#if(DE_PRINT_UDP_PORT!=0)
	udp_data_show(DE_UDP_SEND, udp_print_flag, &maddr, data, len);
	//PRINT_HEX(data, len);
#endif
}
#endif

#if(DE_PRINT_UDP_PORT!=0)
void set_udp_print_flag(uint8 flag)
{
	udp_print_flag = flag;
}

#ifdef DE_TRANS_UDP_STREAM_LOG
void delog_udp_sendto(char *data, int len)
{
	sendto(udpfd, data, len, 0, 
		(struct sockaddr *)&ulog_addr, sizeof(struct sockaddr));
}
#endif

void enable_datalog_atime()
{
	lwflag = 1;
}

void udp_data_show(deudp_print_t deprint, uint8 flag,
		struct sockaddr_in *addr, char *data, int len)
{
	int i;
	for(i=0; i<8; i++)
	{
		switch(i)
		{
		case 0: 
			if((flag&(1<<i)) 
				&& (!strncmp(data, TR_HEAD_PI, 3)
				|| !strncmp(data, TR_HEAD_UL, 3)))
			{
				goto show_end;	
			}
			break;
			
		case 1: 
			if((flag&(1<<i)) 
				&& (!strncmp(data, TR_HEAD_BI, 3)
				|| !strncmp(data, TR_HEAD_SL, 3)))
			{
				goto show_end;	
			}
			break;
			
		case 2: 
			if((flag&(1<<i)) && !strncmp(data, TR_HEAD_GP, 3))
			{
				goto show_end;	
			}
			break;
			
		case 3: 
			if((flag&(1<<i)) && !strncmp(data, TR_HEAD_RP, 3))
			{
				goto show_end;	
			}
			break;
			
		case 4: 
			if((flag&(1<<i)) && !strncmp(data, TR_HEAD_GD, 3))
			{
				goto show_end;	
			}
			break;
			
		case 5: 
			if((flag&(1<<i)) && !strncmp(data, TR_HEAD_RD, 3))
			{
				goto show_end;	
			}
			break;
			
		case 6: 
			if((flag&(1<<i)) && !strncmp(data, TR_HEAD_DC, 3))
			{
				goto show_end;	
			}
			break;
			
		case 7: 
			if((flag&(1<<i)) && !strncmp(data, TR_HEAD_UB, 3))
			{
				goto show_end;	
			}
			break;
		}
	}

#ifdef DE_TRANS_UDP_STREAM_LOG
	if(deprint == DE_UDP_RECV
		&& addr->sin_port == htons(DE_UDP_PORT)
		&& addr->sin_addr.s_addr == inet_addr("127.0.0.1"))
	{
		if(len >= 13 && !memcmp(data, DEU_CMD_PREFIX, 11))
		{
			uint8 flag;
			incode_ctoxs(&flag, data+11, 2);
			set_udp_print_flag(flag);
			DE_PRINTF(1, "\n%s%X succeed\n\n", DEU_CMD_PREFIX, flag);
		}
		else
		{
			DE_PRINTF(0, "\nUnrecognized cmd:%s", data);
			DE_PRINTF(0, "follow:\"%s%s\"\n", DEU_CMD_PREFIX, "value(hex)");
			DE_PRINTF(0, "value:\n");
			DE_PRINTF(0, "  PI:|UL:	01\n");
			DE_PRINTF(0, "  BI:|SL:	02\n");
			DE_PRINTF(0, "  GP:  	04\n");
			DE_PRINTF(0, "  RP:  	08\n");
			DE_PRINTF(0, "  GD:  	10\n");
			DE_PRINTF(0, "  RD:  	20\n");
			DE_PRINTF(0, "  DC:  	40\n");
			DE_PRINTF(0, "  UB:  	80\n\n");
		}
	}
#endif

	return;
	
show_end:
	if(deprint == DE_UDP_SEND)
	{
		DE_PRINTF(lwflag, "UDP:send %d bytes, to ip=%s:%u\n", 
					len, inet_ntoa(addr->sin_addr), 
					ntohs(addr->sin_port));
	}
	else if(deprint == DE_UDP_RECV)
	{
		DE_PRINTF(lwflag, "UDP:receive %d bytes, from ip=%s:%u\n", 
					len, inet_ntoa(addr->sin_addr), 
					ntohs(addr->sin_port));
	}
				
	DE_PRINTF(lwflag, "data:%s\n", data);
	lwflag = 0;
}
#endif

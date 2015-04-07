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
#include <protocol/trframelysis.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#ifdef TRANS_TCP_SERVER
static int tcpfd;
#endif
#ifdef TRANS_UDP_SERVICE
static int udpfd;
static struct sockaddr_in m_addr, server_addr;
#endif

#ifdef TRANS_TCP_SERVER
int get_tcp_fd()
{
	return tcpfd;
}

int socket_tcp_server_init(int port)
{
	struct sockaddr_in server_addr;
	
	if ((tcpfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("tcp socket fail");
		return -1;
	}

	server_addr.sin_family = PF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("192.168.1.1");
	if (bind(tcpfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
	{
		perror("bind tcp ip fail");
		return -1;
	}

	listen(tcpfd, 10);

	select_set(tcpfd);
}

void socket_tcp_client_connect(int fd)
{
	int rw;
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);

	rw = accept(fd, (struct sockaddr *)&client_addr, &len);

	DE_PRINTF("TCP:accept,ip=%s:%u\n\n", 
		inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

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

	select_set(rw);
}

void socket_tcp_client_release(int fd)
{
	close(fd);
	select_clr(fd);

#ifdef TRANS_TCP_CONN_LIST
	tcp_conn_t *m_list = queryfrom_tcpconn_list(fd);
	if(m_list != NULL)
	{
		DE_PRINTF("TCP:release,ip=%s:%u\n\n", 
			inet_ntoa(m_list->client_addr.sin_addr), 
			ntohs(m_list->client_addr.sin_port));
	}
	
	delfrom_tcpconn_list(fd);
#else
	DE_PRINTF("TCP:release,fd=%d\n\n", fd);
#endif
}

void socket_tcp_client_recv(int fd)
{
	int nbytes;
	char buf[MAXSIZE];
	
	memset(buf, 0, sizeof(buf));
   	if ((nbytes = recv(fd, buf, sizeof(buf), 0)) <= 0)
   	{
      	socket_tcp_client_release(fd);
	}
	else
	{
#ifdef TRANS_TCP_CONN_LIST
		tcp_conn_t *m_list = queryfrom_tcpconn_list(fd);
		if(m_list != NULL)
		{
			DE_PRINTF("TCP:receive %d bytes, from ip=%s:%u\n", 
				nbytes, inet_ntoa(m_list->client_addr.sin_addr), 
				ntohs(m_list->client_addr.sin_port));
		}
#endif
		DE_PRINTF("data:%s\n", buf);
	}
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
	m_addr.sin_port = htons(TRANS_UDP_PORT);
	m_addr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("192.168.1.1");

	server_addr.sin_family = PF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
	
	if (bind(udpfd, (struct sockaddr *)&m_addr, sizeof(struct sockaddr)) < 0)
	{
		perror("bind udp ip fail");
		return -1;
	}

	select_set(udpfd);
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

	//DE_PRINTF("UDP:receive %d bytes, from ip=%s:%u\n", 
		//nbytes, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

	//DE_PRINTF("data:%s\n", buf);

#ifdef COMM_CLIENT
	analysis_capps_frame(client_addr, buf, nbytes);
#endif

#ifdef TRANS_UDP_SESS_QUEUE
	addto_udpsess_queue(&client_addr);
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
		printf("erro ip address\n");
		return;
	}
	
	struct sockaddr_in maddr;

	maddr.sin_family = PF_INET;
	maddr.sin_port = htons(iport);
	maddr.sin_addr.s_addr = inet_addr(saddr);
	
	sendto(udpfd, data, len, 0, 
		(struct sockaddr *)&maddr, sizeof(struct sockaddr));

	DE_PRINTF("UDP: send %d bytes to ip=%s:%u\n", \
		len, inet_ntoa(maddr.sin_addr), ntohs(maddr.sin_port));

	DE_PRINTF("data:%s\n\n", data);
}
#endif


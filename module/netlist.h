/*
 * netlist.h
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
#ifndef __NETLIST_H__
#define __NETLIST_H__

#include <services/globals.h>
#include <arpa/inet.h>

#ifdef TRANS_TCP_CONN_LIST
typedef struct TCPConn
{
	int fd;
	struct sockaddr_in client_addr;
	struct TCPConn *next;
}tcp_conn_t;

typedef struct TCPConnList
{
	tcp_conn_t *p_head;
	int num;
	const int max_size;
}tcp_conn_list_t;
#endif

#ifdef TRANS_UDP_SESS_QUEUE
typedef struct UDPSessQueue
{
	int head;
	int tail;
	int num;
	const int max_size;
	struct sockaddr_in client_addrs[TRANS_UDP_SESS_MAX_SIZE];
}udp_sess_queue_t;
#endif

#ifdef TRANS_TCP_CONN_LIST
int addto_tcpconn_list(tcp_conn_t *list);
tcp_conn_t *queryfrom_tcpconn_list(int fd);
int delfrom_tcpconn_list(int fd);
#endif

#ifdef TRANS_UDP_SESS_QUEUE
int addto_udpsess_queue(struct sockaddr_in *addr);
int queryfrom_udpsess_queue(struct sockaddr_in *addr);
#endif

#endif  // __NETLIST_H__
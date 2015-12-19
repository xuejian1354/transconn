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

#ifdef __cplusplus
extern "C" {
#endif

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

int addto_tcpconn_list(tcp_conn_t *list);
tcp_conn_t *queryfrom_tcpconn_list(int fd);
tcp_conn_t *queryfrom_tcpconn_list_with_ipaddr(char *ipaddr);
int delfrom_tcpconn_list(int fd);

#ifdef __cplusplus
}
#endif

#endif  // __NETLIST_H__
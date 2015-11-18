/*
 * request.c
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
#include "request.h"
#include <protocol/protocol.h>
#include <module/balancer.h>
#ifdef DB_API_SUPPORT
#include <module/dbopt.h>
#endif
#include <module/netlist.h>

#ifdef TRANS_UDP_SERVICE
void send_frame_udp_request(char *ipaddr, char *data, int len)
{
	socket_udp_sendto(ipaddr, data, len);
}
#endif

#ifdef TRANS_TCP_CLIENT
void send_frame_tcp_request(char *data, int len)
{
	socket_tcp_client_send(data, len);
}
#endif

#ifdef TRANS_TCP_SERVER
void send_frame_tcp_respond(frhandler_arg_t *arg, char *data, int len)
{
	socket_tcp_server_send(arg, data, len);
}
#endif

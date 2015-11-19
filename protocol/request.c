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
#include <cJSON.h>

#ifdef COMM_CLIENT
char *trans_protocol_request_alloc(char *protocol)
{
	return NULL;
}

char *trans_report_alloc(zidentify_no_t sns[], int len)
{
	return NULL;
}

char *trans_check_alloc(zidentify_no_t sns[], int len)
{
	return NULL;
}

char *trans_respond_alloc(zidentify_no_t sns[], int len)
{
	return NULL;
}

void trans_refresh_handler(char *data)
{}

void trans_control_handler(char *data)
{}

void trans_protocol_respond_handler(char *data)
{}
#endif

#ifdef COMM_SERVER
void trans_protocol_request_handler(char *data)
{}

void trans_report_handler(char *data)
{}

void trans_check_handler(char *data)
{}

void trans_respond_handler(char *data)
{}

char *trans_refresh_alloc(zidentify_no_t sns[], int len)
{
	return NULL;
}

char *trans_control_alloc(zidentify_no_t sns[], int len)
{
	return NULL;
}

char *trans_protocol_respond_alloc(char *protocol)
{}
#endif

#ifdef COMM_CLIENT
void sync_gateway_info(gw_info_t *pgw_info)
{}

void sync_zdev_info(dev_info_t *pdev_info)
{}
#endif

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

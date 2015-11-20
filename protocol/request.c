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
void trans_refresh_handler(trfr_refresh_t *refresh)
{}

void trans_control_handler(trfr_control_t *control)
{}

void trans_protocol_respond_handler(trfr_tocolres_t *tocolres)
{}
#endif

void trans_protocol_request_handler(trfr_tocolreq_t *tocolreq)
{
	if(tocolreq == NULL)
	{
		return;
	}

#ifdef COMM_SERVER
	if(tocolreq->action == ACTION_TOCOLREQ)
	{
		if(!strncmp(tocolreq->protocol, FIELD_TOCOL_UDP, 3))
		{
			
		}
		else if(!strncmp(tocolreq->protocol, FIELD_TOCOL_TCP, 3))
		{
			
		}
		else if(!strncmp(tocolreq->protocol, FIELD_TOCOL_HTTP, 4))
		{
			
		}
	}
#endif
#ifdef COMM_CLIENT
	if(tocolreq->action == ACTION_TOCOLRES)
	{

	}
#endif
}

#ifdef COMM_SERVER
void trans_report_handler(trfr_report_t *report)
{}

void trans_check_handler(trfr_check_t *check)
{}

void trans_respond_handler(trfr_respond_t *respond)
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

/*
 * request.h
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

#ifndef __REQUEST_H__
#define __REQUEST_H__

#include <services/globals.h>
#include <protocol/devalloc.h>
#include <module/netapi.h>

typedef enum
{
	ACTION_REPORT = 1,
	ACTION_CHECK,
	ACTION_REFRESH,
	ACTION_CONTROL,
	ACTION_RESPOND
}trans_action_t;

#ifdef COMM_CLIENT
char *trans_protocol_request_alloc(char *protocol);
char *trans_report_alloc(zidentify_no_t sns[], int len);
char *trans_check_alloc(zidentify_no_t sns[], int len);
char *trans_respond_alloc(zidentify_no_t sns[], int len);
void trans_refresh_handler(char *data);
void trans_control_handler(char *data);
void trans_protocol_respond_handler(char *data);
#endif

#ifdef COMM_SERVER
void trans_protocol_request_handler(char *data);
void trans_report_handler(char *data);
void trans_check_handler(char *data);
void trans_respond_handler(char *data);
char *trans_refresh_alloc(zidentify_no_t sns[], int len);
char *trans_control_alloc(zidentify_no_t sns[], int len);
char *trans_protocol_respond_alloc(char *protocol);
#endif

#ifdef COMM_CLIENT
void sync_gateway_info(gw_info_t *pgw_info);
void sync_zdev_info(dev_info_t *pdev_info);
#endif

#ifdef TRANS_UDP_SERVICE
void send_frame_udp_request(char *ipaddr, char *data, int len);
#endif
#ifdef TRANS_TCP_CLIENT
void send_frame_tcp_request(char *data, int len);
#endif
#ifdef TRANS_TCP_SERVER
void send_frame_tcp_respond(frhandler_arg_t *arg, char *data, int len);
#endif

#endif  //__REQUEST_H__
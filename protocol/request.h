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
#include <arpa/inet.h>
#include <sys/socket.h>
#include <protocol/devalloc.h>

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
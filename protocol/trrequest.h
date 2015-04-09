/*
 * trrequest.h
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

#ifndef __TRREQUEST_H__
#define __TRREQUEST_H__

#include <services/globals.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <protocol/trframelysis.h>

void pi_handler(struct sockaddr_in *addr, pi_t *pi);
void bi_handler(struct sockaddr_in *addr, bi_t *bi);

void send_pi_udp_request(char *ipaddr, tr_frame_type_t trfra, char *data, int len);
void send_bi_udp_respond(char *ipaddr, tr_frame_type_t trfra, char *data, int len);
#endif  //__TRREQUEST_H__
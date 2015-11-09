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
#include <protocol/trframelysis.h>
#include <protocol/devalloc.h>

void pi_handler(frhandler_arg_t *arg, pi_t *p_pi);
void bi_handler(frhandler_arg_t *arg, bi_t *p_bi);

void ul_handler(frhandler_arg_t *arg, ul_t *p_ul);
void sl_handler(frhandler_arg_t *arg, sl_t *p_sl);

void gp_handler(frhandler_arg_t *arg, gp_t *p_gp);
void rp_handler(frhandler_arg_t *arg, rp_t *p_rp);

void gd_handler(frhandler_arg_t *arg, gd_t *p_gd);
void rd_handler(frhandler_arg_t *arg, rd_t *p_rd);

void dc_handler(frhandler_arg_t *arg, dc_t *p_dc);
void ub_handler(frhandler_arg_t *arg, ub_t *p_ub);

#ifdef TRANS_UDP_SERVICE
void send_frame_udp_request(char *ipaddr, tr_head_type_t htype, void *frame);
#endif
#ifdef TRANS_TCP_CLIENT
void send_frame_tcp_request(tr_head_type_t htype, void *frame);
#endif
#ifdef TRANS_TCP_SERVER
void send_frame_tcp_respond(frhandler_arg_t *arg, tr_head_type_t htype, void *frame);
#endif

#endif  //__REQUEST_H__
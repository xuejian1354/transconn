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

void pi_handler(struct sockaddr_in *addr, pi_t *p_pi);
void bi_handler(struct sockaddr_in *addr, bi_t *p_bi);

void ul_handler(struct sockaddr_in *addr, ul_t *p_ul);
void sl_handler(struct sockaddr_in *addr, sl_t *p_sl);

void gp_handler(struct sockaddr_in *addr, gp_t *p_gp);
void rp_handler(struct sockaddr_in *addr, rp_t *p_rp);

void gd_handler(struct sockaddr_in *addr, gd_t *p_gd);
void rd_handler(struct sockaddr_in *addr, rd_t *p_rd);

void dc_handler(struct sockaddr_in *addr, dc_t *p_dc);
void ub_handler(struct sockaddr_in *addr, ub_t *p_ub);

void send_frame_udp_request(char *ipaddr, tr_head_type_t htype, void *frame);

#endif  //__REQUEST_H__
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
void gp_handler(struct sockaddr_in *addr, gp_t *gp);
void rp_handler(struct sockaddr_in *addr, rp_t *rp);
void gd_handler(struct sockaddr_in *addr, gd_t *gd);
void rd_handler(struct sockaddr_in *addr, rd_t *rd);
void dc_handler(struct sockaddr_in *addr, dc_t *dc);
void ub_handler(struct sockaddr_in *addr, ub_t *ub);

void send_pi_udp_request(char *ipaddr, 
	tr_frame_type_t trfra, char *data, int len, uint8 *sn);
void send_bi_udp_respond(char *ipaddr, 
	tr_frame_type_t trfra, char *data, int len, uint8 *sn);
void send_gp_udp_request(char *ipaddr, 
	zidentify_no_t zidentify_no, cidentify_no_t cidentify_no, char *data, int len);
void send_rp_udp_respond(char *ipaddr, 
	zidentify_no_t zidentify_no, cidentify_no_t cidentify_no, char *data, int len);
void send_gd_udp_request(char *ipaddr, 
	zidentify_no_t zidentify_no, cidentify_no_t cidentify_no);
void send_rd_udp_respond(char *ipaddr,
	zidentify_no_t zidentify_no, cidentify_no_t cidentify_no);
void send_dc_udp_request(char *ipaddr, char *data, int len);
void send_ub_udp_respond(char *ipaddr, char *data, int len);
#endif  //__TRREQUEST_H__

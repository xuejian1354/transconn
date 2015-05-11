/*
 * protocol.h
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
#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <services/globals.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <protocol/framelysis.h>
#include <protocol/trframelysis.h>
#include <protocol/devopt.h>

#define GATEWAY_BUFFER_FIX_SIZE		32
#define ZDEVICE_BUFFER_SIZE		23

typedef struct Dev_Info
{
	zidentify_no_t zidentity_no;
	uint16 znet_addr;
	fr_app_type_t zapp_type;
	fr_net_type_t znet_type;
	dev_opt_t *zdev_opt;
	struct Dev_Info *next;
}dev_info_t;


typedef struct Gw_Info
{
	zidentify_no_t gw_no;
	uint16 zpanid;
	uint16 zchannel;
	uint32 rand;
	tr_trans_type_t trans_type;
	uint8 ipaddr[IP_ADDR_MAX_SIZE];
	uint8 ip_len;
	uint8 serverip_addr[IP_ADDR_MAX_SIZE];
	uint8 serverip_len;
	pthread_mutex_t lock;
	dev_info_t *p_dev;
	struct Gw_Info *next;
}gw_info_t;

typedef struct
{
	gw_info_t *p_gw;
	pthread_mutex_t lock;
	long max_num;
}gw_list_t;

typedef struct Cli_Info
{
	cidentify_no_t cidentify_no;
	tr_trans_type_t trans_type;
	uint8 ipaddr[IP_ADDR_MAX_SIZE];
	uint8 ip_len;
	uint8 serverip_addr[IP_ADDR_MAX_SIZE];
	uint8 serverip_len;
	int check_count;
	int check_conn;
	struct Cli_Info *next;
}cli_info_t;

typedef struct
{
	cli_info_t *p_cli;
	pthread_mutex_t lock;
	long max_num;
}cli_list_t;

typedef struct
{
	int fd;
	struct sockaddr_in addr; 
	char *buf;
	int len;
}frhandler_arg_t;

uint8 *get_zdev_buffer_alloc(dev_info_t *dev_info);
void get_zdev_buffer_free(uint8 *p);
dev_info_t *get_zdev_frame_alloc(uint8 *buffer, int length);
void get_zdev_frame_free(dev_info_t *p);

uint8 *get_gateway_buffer_alloc(gw_info_t *gw_info);
void get_gateway_buffer_free(uint8 *p);
gw_info_t *get_gateway_frame_alloc(uint8 *buffer, int length);
void get_gateway_frame_free(gw_info_t *p);

int add_zdev_info(gw_info_t *gw_info, dev_info_t *m_dev);
dev_info_t *query_zdev_info(gw_info_t *gw_info, uint16 znet_addr);
int del_zdev_info(gw_info_t *gw_info, uint16 znet_addr);

frhandler_arg_t *get_frhandler_arg_alloc(int fd, struct sockaddr_in *addr, 
														char *buf, int len);
void get_frhandler_arg_free(frhandler_arg_t *arg);

#ifdef COMM_CLIENT
gw_info_t *get_gateway_info();

int add_zdevice_info(dev_info_t *m_dev);
dev_info_t *query_zdevice_info(uint16 znet_addr);
dev_info_t *query_zdevice_info_with_sn(zidentify_no_t zidentify_no);
int del_zdevice_info(uint16 znet_addr);
#endif

cli_list_t *get_client_list();

int add_client_info(cli_info_t *m_info);
cli_info_t *query_client_info(cidentify_no_t cidentify_no);
int del_client_info(cidentify_no_t cidentify_no);


#ifdef COMM_SERVER
gw_list_t *get_gateway_list();

int add_gateway_info(gw_info_t *m_gw);
gw_info_t *query_gateway_info(zidentify_no_t gw_no);
int del_gateway_info(zidentify_no_t gw_no);
#endif


#ifdef COMM_CLIENT
void analysis_zdev_frame(frhandler_arg_t *arg);
#endif
void analysis_capps_frame(frhandler_arg_t *arg);

fr_buffer_t *get_switch_buffer_alloc(fr_head_type_t head_type, 
	dev_opt_t *opt, void *frame);

#endif  //__PROTOCOL_H__

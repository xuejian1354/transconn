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

#include <mconfig.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <protocol/framelysis.h>

typedef struct Dev_Info
{
	zidentify_no_t zidentity_no;
	uint16 znet_addr;
	fr_app_type_t zapp_type;
	fr_net_type_t znet_type;
	struct Dev_Info *next;
}dev_info_t;


typedef struct
{
	zidentify_no_t gw_no;
	uint16 zpanid;
	uint16 zchannel;
	pthread_mutex_t lock;
	dev_info_t *p_dev;
}gw_info_t;

typedef struct Cli_Info
{
	cidentify_no_t cidentify_no;
	struct sockaddr_in sock_addr;
	struct Cli_Info *next;
}cli_info_t;

typedef struct
{
	cli_info_t *cli_info;
	int max_num;
}cli_list_t;

gw_info_t *get_gateway_info();

#ifdef COMM_CLIENT
void analysis_zdev_frame(char *buf, int len);
void analysis_capps_frame(struct sockaddr_in *addr, char *buf, int len);
#endif

#endif  //__PROTOCOL_H__

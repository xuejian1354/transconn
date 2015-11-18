/*
 * devalloc.h
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
#ifndef __DEVALLOC_H__
#define __DEVALLOC_H__

#include <services/globals.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <protocol/framelysis.h>
#include <protocol/devopt.h>
#include <protocol/users.h>

#define GATEWAY_BUFFER_FIX_SIZE		34
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
	fr_app_type_t zapp_type;
	uint16 zpanid;
	uint16 zchannel;
	dev_opt_t *zgw_opt;
	uint32 rand;
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
	int fd;
	struct sockaddr_in addr; 
	char *buf;
	int len;
}frhandler_arg_t;

typedef struct
{
	gw_info_t *p_gw;
	pthread_mutex_t lock;
	long max_num;
}gw_list_t;

uint8 *get_zdev_buffer_alloc(dev_info_t *dev_info);
void get_zdev_buffer_free(uint8 *p);
dev_info_t *get_zdev_frame_alloc(uint8 *buffer, int length);
void get_zdev_frame_free(dev_info_t *p);

fr_buffer_t *get_gateway_buffer_alloc(gw_info_t *gw_info);
void get_gateway_buffer_free(uint8 *p);

gw_info_t *get_gateway_frame_alloc(uint8 *buffer, int length);
void get_gateway_frame_free(gw_info_t *p);

frhandler_arg_t *get_frhandler_arg_alloc(int fd, struct sockaddr_in *addr, 
														char *buf, int len);
void get_frhandler_arg_free(frhandler_arg_t *arg);


fr_buffer_t *get_switch_buffer_alloc(fr_head_type_t head_type, 
	dev_opt_t *opt, void *frame);

#endif  //__DEVALLOC_H__


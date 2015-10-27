/*
 * balancer.h
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
#ifndef __BALANCER_H__
#define __BALANCER_H__

#include <services/globals.h>

//#define LOAD_BALANCE_SUPPORT

#define RELSER_NAME_NUM		64

typedef enum
{
	LINE_NAME,
	LINE_IP,
	LINE_NONE
}line_data_type_t;

typedef struct
{
	line_data_type_t type;
	char data[64];
}line_data_t;

typedef struct RelSer_info
{
	char name[RELSER_NAME_NUM];
	uint8 ipaddr[IP_ADDR_MAX_SIZE];
	int gw_num;
	int cli_num;
	struct RelSer_info *next;
}relser_info_t;

typedef struct RelSer_List
{
	relser_info_t *p_ser;
	int max_num;
}relser_list_t;

int serlist_read_from_confile(void);
const char *get_server_ip(void);
const char *get_server_ip_from_name(char *name);
const char *get_server_name_from_ip(char *ip);

#endif	//__BALANCER_H__
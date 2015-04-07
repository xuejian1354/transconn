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
#include <protocol/framelysis.h>

typedef byte zidentify_no_t[8];

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
	zidentify_no_t zidentity_no;
	uint16 zpanid;
	uint16 zchannel;
	dev_info_t *p_dev;
}gw_info_t;

#ifdef COMM_CLIENT
void analysis_ssa_frame(char *buf, int len);
#endif

#endif  //__PROTOCOL_H__

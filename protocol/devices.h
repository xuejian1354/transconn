/*
 * devices.h
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
#ifndef __DEVICES_H__
#define __DEVICES_H__

#include <services/globals.h>
#include <protocol/common/fieldlysis.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Dev_Info
{
	zidentify_no_t dev_no;
	fr_app_type_t type;
	uint32 data;
	uint8 ischange;
	struct Dev_Info *next;
}dev_info_t;

typedef struct Gw_Info
{
	zidentify_no_t gw_no;
	fr_app_type_t type;
	pthread_mutex_t lock;
	dev_info_t *p_dev;
	struct Gw_Info *next;
}gw_info_t;


int add_zdev_info(gw_info_t *gw_info, dev_info_t *m_dev);
dev_info_t *query_zdev_info(gw_info_t *gw_info, char *dev_no);
int del_zdev_info(gw_info_t *gw_info, zidentify_no_t dev_no);

gw_info_t *get_gateway_info();

int add_zdevice_info(dev_info_t *m_dev);
dev_info_t *query_zdevice_info(zidentify_no_t dev_no);
int del_zdevice_info(zidentify_no_t dev_no);

#ifdef __cplusplus
}
#endif

#endif  //__DEVICES_H__


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
#include <protocol/old/devalloc.h>
#include <protocol/common/fieldlysis.h>

#ifdef __cplusplus
extern "C" {
#endif

int add_zdev_info(gw_info_t *gw_info, dev_info_t *m_dev);
dev_info_t *query_zdev_info(gw_info_t *gw_info, uint16 znet_addr);
int del_zdev_info(gw_info_t *gw_info, uint16 znet_addr);


#ifdef COMM_CLIENT
gw_info_t *get_gateway_info();

int add_zdevice_info(dev_info_t *m_dev);
dev_info_t *query_zdevice_info(uint16 znet_addr);
dev_info_t *query_zdevice_info_with_sn(zidentify_no_t zidentify_no);
uint16 get_znet_addr_with_sn(sn_t sn);
int del_zdevice_info(uint16 znet_addr);
#endif

#ifdef COMM_SERVER
gw_list_t *get_gateway_list();

int add_gateway_info(gw_info_t *m_gw);
gw_info_t *query_gateway_info(zidentify_no_t gw_no);
int del_gateway_info(zidentify_no_t gw_no);
#endif

#ifdef __cplusplus
}
#endif

#endif  //__DEVICES_H__


/*
 * dbclient.h
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
#ifndef __DBCLIENT_H__
#define __DBCLIENT_H__

#include <services/globals.h>
#include <protocol/old/devices.h>
#include <sqlite3.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DB_API_WITH_SQLITE
int sqlclient_init();
void sqlclient_release();

int sqlclient_add_zdev(gw_info_t *p_gw, dev_info_t *m_dev);
int sqlclient_query_zdev(gw_info_t *p_gw, zidentify_no_t zidentity_no);
int sqlclient_del_zdev(gw_info_t *p_gw, zidentify_no_t zidentity_no);

int sqlclient_uponline_zdev(gw_info_t *p_gw, 
			uint8 isonline , uint16* znet_addrs, int addrs_len);
int sqlclient_set_datachange_zdev(zidentify_no_t dev_no, uint8 ischange);

int sqlclient_add_gateway(gw_info_t *m_gw);
int sqlclient_query_gateway(zidentify_no_t gw_no);
int sqlclient_del_gateway(zidentify_no_t gw_no);

void sqlclient_get_zdevices(uint8 isrefresh, trfield_device_t ***devices, int *dev_size);
void sqlclient_get_devdatas(char **text, long *text_len);
#endif

#ifdef __cplusplus
}
#endif

#endif	//__DBCLIENT_H__
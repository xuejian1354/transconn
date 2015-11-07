/*
 * dbopt.h
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
#ifndef __DBOPT_H__
#define __DBOPT_H__

#include <services/globals.h>
#include <protocol/protocol.h>
#include <mysql/mysql.h>

pthread_mutex_t *get_sql_add_lock();

int sql_init();
int sql_reconnect();
void sql_release();
int sql_isuse();

int sql_add_zdev(gw_info_t *p_gw, dev_info_t *m_dev);
dev_info_t *sql_query_zdev(gw_info_t *p_gw, zidentify_no_t zidentity_no);
int sql_del_zdev(gw_info_t *p_gw, zidentify_no_t zidentity_no);

int sql_uponline_zdev(gw_info_t *p_gw, 
			uint8 isonline , uint16* znet_addrs, int addrs_len);

int sql_add_gateway(gw_info_t *m_gw);
int sql_query_gateway(zidentify_no_t gw_no);
int sql_del_gateway(zidentify_no_t gw_no);

int set_user_info_from_sql(char *email, cli_user_t *user_info);
int set_zdev_to_user_sql(char *email, char *dev_str);

void sql_test();

#endif	//__DBOPT_H__
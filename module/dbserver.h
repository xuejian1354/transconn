/*
 * dbserver.h
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
#ifndef __DBSERVER_H__
#define __DBSERVER_H__

#include <services/globals.h>
#include <protocol/protocol.h>
#include <strings_t.h>
#include <mysql/mysql.h>
#ifdef DB_API_WITH_MYSQL

int sqlserver_init();
int sqlserver_reconnect();
 void sqlserver_release();
 int sqlserver_isuse();
 int sqlserver_add_zdevices(frhandler_arg_t *arg, trfr_report_t *report);
 int sqlserver_update_zdevice(frhandler_arg_t *arg, trfr_respond_t *respond);
 int sqlserver_query_zdevice(gw_info_t *p_gw, zidentify_no_t zidentity_no);
 int sqlserver_del_zdevice(gw_info_t *p_gw, zidentify_no_t zidentity_no);

int get_user_info_from_sql(char *email, cli_user_t *user_info);
void sync_user_info_to_sql(char *data);
void del_user_info_to_sql(char *data);
int set_device_to_user_sql(char *email, char *dev_str);
#endif

#endif	//__DBSERVER_H__
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
#include <protocol/common/fieldlysis.h>
#include <module/netapi.h>
#include <strings_t.h>
#include <mysql/mysql.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DB_API_WITH_MYSQL

int sqlserver_init();
int sqlserver_reconnect();
 void sqlserver_release();
 int sqlserver_isuse();

int sqlserver_add_gateway(frhandler_arg_t *arg, sn_t gwsn);
int sqlserver_query_gateway(sn_t gwsn);
char *sqlserver_get_column_from_gwsn(char *field, sn_t gwsn);
 int sqlserver_add_zdevices(frhandler_arg_t *arg, trfr_report_t *report);
 int sqlserver_update_zdevice(frhandler_arg_t *arg, trfr_respond_t *respond);
 int sqlserver_query_zdevice(sn_t serno);
 char *sqlserver_get_column_from_zdevice(char *field, sn_t serno);
 int sqlserver_del_zdevice(sn_t serno);

int get_user_info_from_sql(char *email, cli_user_t *user_info);
void sync_user_info_to_sql(char *data);
void del_user_info_to_sql(char *data);
int set_device_to_user_sql(char *email, char *dev_str);
#endif

#ifdef __cplusplus
}
#endif

#endif	//__DBSERVER_H__
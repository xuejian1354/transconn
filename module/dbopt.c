/*
 * dbopt.c
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
#include "dbopt.h"

#define DB_SERVER	"localhost"
#define DB_PORT		3306

extern global_conf_t g_conf;

void sql_test()
{
     MYSQL mysql_conn;

     if (mysql_init(&mysql_conn) != NULL)
     {
         if (mysql_real_connect(&mysql_conn, DB_SERVER, g_conf.db_user, 
		 	g_conf.db_password, g_conf.db_name, 0, NULL, 0) != NULL)
		 {
		 	printf("%s:GOOD!\n", __func__);
         }
		 else 
		 {
		 	printf("%s:Connection fails.\n", __func__);
		 }
      }
	 else
	 {
	 	printf("%s:Initialization fails.\n", __func__);
	 }

     mysql_close(&mysql_conn);
}
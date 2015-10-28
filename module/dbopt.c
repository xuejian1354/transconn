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

int Sql_AddZdev(dev_info_t *m_dev)
{
	return 0;
}

dev_info_t *Sql_QueryZdev(zidentify_no_t zidentity_no)
{
	return NULL;
}

int Sql_DelZdev(zidentify_no_t zidentity_no)
{
	return 0;
}

int Sql_AddGateway(gw_info_t *m_gw)
{
	return 0;
}

gw_info_t *Sql_QueryGateway(zidentify_no_t gw_no)
{
	return NULL;
}

int Sql_DelGateway(zidentify_no_t gw_no)
{
	return 0;
}

void Sql_Test()
{
     MYSQL mysql_conn;
	 global_conf_t *m_conf = get_global_conf();

     if (mysql_init(&mysql_conn) != NULL)
     {
         if (mysql_real_connect(&mysql_conn, DB_SERVER, m_conf->db_user, 
		 		m_conf->db_password, m_conf->db_name, 0, NULL, 0) != NULL)
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
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

#define GET_CMD_LINE()	cmdline

#define SET_CMD_LINE(format, args...)  	\
st(  									\
	bzero(cmdline, sizeof(cmdline));  	\
	sprintf(cmdline, format, ##args);  	\
)

static MYSQL mysql_conn;
static int is_userful = 0;

static char cmdline[256];

int sql_init()
{	 global_conf_t *m_conf = get_global_conf();

     if (mysql_init(&mysql_conn) == NULL)
     {
	 	printf("%s()%d: sql init fails\n", __FUNCTION__, __LINE__);
		return -1;
     }

	 is_userful = 1;
	 
     if (mysql_real_connect(&mysql_conn, DB_SERVER, m_conf->db_user, 
	 		m_conf->db_password, m_conf->db_name, 0, NULL, 0) == NULL)
     {
	 	SET_CMD_LINE("%s %s", "CREATE DATABASE", m_conf->db_name);
		if(mysql_query(&mysql_conn, GET_CMD_LINE()))
		{
			printf("GET_CMD_LINE() : %s\n", GET_CMD_LINE());
			printf("%s()%d : sql create \"%s\" fails\n", 
				__FUNCTION__, __LINE__, m_conf->db_name);
			return -1;
		}
		
		printf("%s()%d : sql create \"%s\"\n", 
				__FUNCTION__, __LINE__, m_conf->db_name);

		if (mysql_real_connect(&mysql_conn, DB_SERVER, m_conf->db_user, 
	 		m_conf->db_password, m_conf->db_name, 0, NULL, 0) == NULL)
		{
			printf("%s()%d : sql connect \"%s\" fails\n", 
					__FUNCTION__, __LINE__, m_conf->db_name);	
			return -1;
		}
	 }

	 DE_PRINTF("%s()%d : sql connect \"%s\"\n", 
	 			__FUNCTION__, __LINE__, m_conf->db_name);	

	 return 0;
}

void sql_release()
{
	is_userful = 0;
	DE_PRINTF("%s()%d : sql close \"%s\"\n", 
	 			__FUNCTION__, __LINE__, get_global_conf()->db_name);	
	mysql_close(&mysql_conn);
}

int sql_isuse()
{
	return is_userful;
}

int sql_add_zdev(dev_info_t *m_dev)
{
	return 0;
}

dev_info_t *sql_query_zdev(zidentify_no_t zidentity_no)
{
	return NULL;
}

int sql_del_zdev(zidentify_no_t zidentity_no)
{
	return 0;
}

int sql_add_gateway(gw_info_t *m_gw)
{
	return 0;
}

gw_info_t *sql_query_gateway(zidentify_no_t gw_no)
{
	return NULL;
}

int sql_gel_gateway(zidentify_no_t gw_no)
{
	return 0;
}

void sql_test()
{
     MYSQL mysql_conn;
	 global_conf_t *m_conf = get_global_conf();

     if (mysql_init(&mysql_conn) != NULL)
     {
         if (mysql_real_connect(&mysql_conn, DB_SERVER, m_conf->db_user, 
		 		m_conf->db_password, m_conf->db_name, 0, NULL, 0) != NULL)
		 {
		 	printf("%s()%d : sql connection OK!\n", __FUNCTION__, __LINE__);
         }
		 else 
		 {
		 	printf("%s()%d : sql connection fails.\n", __FUNCTION__, __LINE__);
		 }
      }
	 else
	 {
	 	printf("%s()%d : sql initialization fails.\n", __FUNCTION__, __LINE__);
	 }

     mysql_close(&mysql_conn);
}

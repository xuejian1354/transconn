/*
 * dbops.c
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
#ifdef DB_API_SUPPORT
#include "dbopt.h"
#include <cJSON.h>

#define DB_SERVER	"localhost"
#define DB_PORT		3306

#ifdef DB_API_WITH_MYSQL
static MYSQL mysql_conn;
static MYSQL_RES *mysql_res;
static MYSQL_ROW mysql_row;
#endif

static pthread_mutex_t sql_lock;

static char is_userful = 0;
char cmdline[CMDLINE_SIZE];

static int sql_excute_cmdline(char *cmdline);

int sql_init()
{
#ifdef DB_API_WITH_MYSQL
	global_conf_t *m_conf = get_global_conf();

	if (mysql_init(&mysql_conn) == NULL)
	{
		DE_PRINTF(1, "%s()%d: mysql init failed\n", __FUNCTION__, __LINE__);
		return -1;
	}

	is_userful = 1;

	mysql_options(&mysql_conn, MYSQL_OPT_RECONNECT, &is_userful);

	if (mysql_real_connect(&mysql_conn, DB_SERVER, m_conf->db_user, 
		m_conf->db_password, m_conf->db_name, 0, NULL, 0) == NULL)
	{
		DE_PRINTF(1, "%s()%d : mysql connect \"%s\" failed\n", 
			__FUNCTION__, __LINE__, m_conf->db_name);
		return -1;
	}

	DE_PRINTF(1, "mysql connect \"%s\"\n\n", m_conf->db_name);

	if(mysql_set_character_set(&mysql_conn, "utf8"))
	{
		DE_PRINTF(1, "%s()%d : mysql character set failed\n", __FUNCTION__, __LINE__);
		return -1;
	} 
#endif

	if(pthread_mutex_init(&sql_lock, NULL) != 0)
	{
		DE_PRINTF(1, "%s()%d : pthread_mutext_init failed\n", __FUNCTION__, __LINE__);
        return -1;
	}

	return 0;
}

int sql_reconnect()
{
#ifdef DB_API_WITH_MYSQL
	global_conf_t *m_conf = get_global_conf();

	if(mysql_ping(&mysql_conn))
	{
		is_userful = 0;
		if (mysql_real_connect(&mysql_conn, DB_SERVER, m_conf->db_user, 
			m_conf->db_password, m_conf->db_name, 0, NULL, 0) == NULL)
		{
			DE_PRINTF(1, "%s()%d : mysql connect \"%s\" failed\n", 
				__FUNCTION__, __LINE__, m_conf->db_name);
			return -1;
		}

		DE_PRINTF(1, "%s()%d : mysql connect \"%s\"\n", 
			__FUNCTION__, __LINE__, m_conf->db_name);

		if(mysql_set_character_set(&mysql_conn, "utf8"))
		{
			DE_PRINTF(1, "%s()%d : mysql character set failed\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		is_userful = 1;
		return 1;
	}
#endif

	return 0;
}

 int sql_excute_cmdline(char *cmdline)
 {
	pthread_mutex_lock(&sql_lock);
	if(sql_reconnect() < 0)
	{
		pthread_mutex_unlock(&sql_lock);
	   	return -1;
	}

#ifdef DB_API_WITH_MYSQL
	if( mysql_query(&mysql_conn, cmdline))
    {
		pthread_mutex_unlock(&sql_lock);
       	DE_PRINTF(1, "%s()%d : mysql query failed\n\n", __FUNCTION__, __LINE__);
	   	return -1;
    }
#endif

	pthread_mutex_unlock(&sql_lock);
	return 0;
 }

void sql_release()
{
	is_userful = 0;
#ifdef DB_API_WITH_MYSQL
	DE_PRINTF(1, "%s()%d : mysql close \"%s\"\n",
	 			__FUNCTION__, __LINE__, get_global_conf()->db_name);
	mysql_close(&mysql_conn);
#endif
}

int sql_isuse()
{
	return is_userful;
}

int sql_add_zdevices(frhandler_arg_t *arg, trfr_report_t *report)
{
	char serno[24] = {0};
	incode_xtocs(serno, m_dev->zidentity_no, sizeof(zidentify_no_t));

	char gwno[24] = {0};
	incode_xtocs(gwno, p_gw->gw_no, sizeof(zidentify_no_t));

	fr_buffer_t *frbuffer = get_devopt_data_tostr(m_dev->zdev_opt);
	char data[24] = {0};
	if(frbuffer != NULL)
	{
		memcpy(data, frbuffer->data, frbuffer->size);
		get_buffer_free(frbuffer);
	}

	if(sql_query_zdev(p_gw, m_dev->zidentity_no) == 0)
	{
		SET_CMD_LINE("%s%04X%s%s%s%d%s%s%s%s%s%s%s%s%s%s%s%s%s", 
				"UPDATE devices SET shortaddr=\'",
				m_dev->znet_addr,
				"\', apptype=\'",
				apptype_str,
				"\', isonline=\'1\', ischange=\'",
				m_dev->isdata_change,
				"\', updatetime=\'",
				get_current_time(),
				"\', data=\'",
				data,
				"\', ipaddr=\'",
				p_gw->ipaddr,
				"\', updated_at=\'",
				get_current_time(),
				"\', gwsn=\'",
				gwno,
				"\' WHERE serialnum=\'",
				serno,
				"\'");

		DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());

		return sql_excute_cmdline(GET_CMD_LINE());
	}

	SET_CMD_LINE("%s%s%s%s%s%s%s%04X%s%s%s%s%s%s%s%s%s%s%s%d%s%s%s%s%s%s%s", 
		"INSERT INTO devices (id, serialnum, apptype, shortaddr, ipaddr, ",
		"commtocol, gwsn, name, area, updatetime, isonline, iscollect, ",
		"ispublic, ischange, users, data, created_at, updated_at) VALUES (NULL, \'",
		serno,
		"\', \'",
		apptype_str,
		"\', \'",
		m_dev->znet_addr,
		"\', \'",
		p_gw->ipaddr,
		"\', \'01\', \'",
		gwno,
		"\', \'",
		get_mix_name(m_dev->zapp_type, 
			p_gw->gw_no[sizeof(zidentify_no_t)-1], 
			m_dev->zidentity_no[sizeof(zidentify_no_t)-1]),
		"\', \'",
		NO_AREA,
		"\', \'",
		get_current_time(),
		"\', \'1\', \'0\', \'0\', \'",
		m_dev->isdata_change,
		"\', \'\', \'",
		data,
		"\', \'",
		get_current_time(),
		"\', \'",
		get_current_time(),
		"\')");

	DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());

	return sql_excute_cmdline(GET_CMD_LINE());
}

int sql_update_zdevice(frhandler_arg_t *arg, trfr_respond_t *respond)
{
	
}

int sql_query_zdevice(gw_info_t *p_gw, zidentify_no_t zidentity_no)
{
	char serstr[24] = {0};
	incode_xtocs(serstr, zidentity_no, sizeof(zidentify_no_t));
	SET_CMD_LINE("%s%s%s", 
		"SELECT * FROM devices WHERE serialnum=\'", 
		serstr, 
		"\'");

#ifdef DB_API_WITH_MYSQL
	//DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
	if(sql_excute_cmdline(GET_CMD_LINE()) < 0)
	{
		return -1;
	}

	if((mysql_res = mysql_store_result(&mysql_conn)) == NULL)
	{
		DE_PRINTF(1, "%s()%d : mysql store result failed\n\n", __FUNCTION__, __LINE__);
		return -1;
	}

    while((mysql_row = mysql_fetch_row(mysql_res)))
    {
		int nums = mysql_num_fields(mysql_res);
		if(nums > 0)
		{
			mysql_free_result(mysql_res);
			return 0;
		}
	}
	mysql_free_result(mysql_res);
#endif

	return 1;
}

int sql_del_zdevice(gw_info_t *p_gw, zidentify_no_t zidentity_no)
{
	char serno[24] = {0};
	incode_xtocs(serno, zidentity_no, sizeof(zidentify_no_t));

	SET_CMD_LINE("%s%s%s", "DELETE FROM devices WHERE serialnum=\'",
		serno,
		"\'");

	//DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
	return sql_excute_cmdline(GET_CMD_LINE());
}
#endif


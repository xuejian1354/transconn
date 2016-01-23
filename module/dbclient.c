/*
 * dbclient.c
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
#include "dbclient.h"
#include <cJSON.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DB_API_WITH_SQLITE

extern char cmdline[CMDLINE_SIZE];

static sqlite3 *sqlite_db;
static char *errmsg;

static pthread_mutex_t sqlclient_lock;

static int sqlclient_excute_cmdline(char *cmdline);


int sqlclient_init()
{
	global_conf_t *m_conf = get_global_conf();
	char db_path[64] = {0};

	sprintf(db_path, "/mnt/%s.db", m_conf->db_name);
	if(sqlite3_open(db_path, &sqlite_db) != SQLITE_OK)
	{
		DE_PRINTF(1, "%s()%d : sqlite connection failed.\n", __FUNCTION__, __LINE__);
		DE_PRINTF(1, "%s\n", sqlite3_errmsg(sqlite_db));
		sqlite3_close(sqlite_db);
		return -1;
	}

	DE_PRINTF(1, "sqlite connect \"%s\"\n\n", m_conf->db_name);

	SET_CMD_LINE("CREATE TABLE gateways (%s%s%s%s%s%s%s%s)",
					"id INTEGER PRIMARY KEY AUTOINCREMENT, ",
					"gwsn VARCHAR(32), ",
					"apptype VARCHAR(8), ",
					"ipaddr VARCHAR(24), ",
					"name VARCHAR(255), ",
					"data TEXT, ",
					"created_at VARCHAR(64), ",
					"updated_at VARCHAR(64)");

	if(sqlite3_exec(sqlite_db, GET_CMD_LINE(), NULL, NULL, &errmsg) != SQLITE_OK)
	{
		/*DE_PRINTF(1, "%s()%d : sqlite create table \"gateways\" failed\n", __FUNCTION__, __LINE__);
		DE_PRINTF(1, "%s()%d : %s\n",  __FUNCTION__, __LINE__, sqlite3_errmsg(sqlite_db));
		sqlite3_close(sqlite_db);
		return -1;*/
	}

	SET_CMD_LINE("CREATE TABLE devices (%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s)",
					"id INTEGER PRIMARY KEY AUTOINCREMENT, ",
					"serialnum VARCHAR(32), ",
					"apptype VARCHAR(8), ",
					"shortaddr VARCHAR(8), ",
					"ipaddr VARCHAR(24), ",
					"commtocol VARCHAR(4), ",
					"gwsn VARCHAR(32), ",
					"name VARCHAR(64), ",
					"area VARCHAR(64), ",
					"updatetime VARCHAR(64), ",
					"isonline TINYINT(1), ",
					"iscollect TINYINT(1), ",
					"ispublic TINYINT(1), ",
					"ischange TINYINT(1), ",
					"users TEXT, ",
					"data TEXT, ",
					"created_at VARCHAR(64), ",
					"updated_at VARCHAR(64)");

	if(sqlite3_exec(sqlite_db, GET_CMD_LINE(), NULL, NULL, &errmsg) != SQLITE_OK)
	{
		/*DE_PRINTF(1, "%s()%d : sqlite create table \"devicess\" failed\n", __FUNCTION__, __LINE__);
		DE_PRINTF(1, "%s()%d : %s\n",  __FUNCTION__, __LINE__, sqlite3_errmsg(sqlite_db));
		sqlite3_close(sqlite_db);
		return -1;*/
	}

	if(pthread_mutex_init(&sqlclient_lock, NULL) != 0)
	{
		DE_PRINTF(1, "%s()%d : pthread_mutext_init failed\n", __FUNCTION__, __LINE__);
        return -1;
	}

	return 0;
}

 int sqlclient_excute_cmdline(char *cmdline)
 {
	pthread_mutex_lock(&sqlclient_lock);

	char *errmsg;
	if(sqlite3_exec(sqlite_db, cmdline, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		pthread_mutex_unlock(&sqlclient_lock);
		DE_PRINTF(1, "%s()%d : sqlite exec failed\n", __FUNCTION__, __LINE__);
		DE_PRINTF(1, "%s()%d : %s\n\n",  __FUNCTION__, __LINE__, sqlite3_errmsg(sqlite_db));
		return -1;
	}

	pthread_mutex_unlock(&sqlclient_lock);
	return 0;
 }

void sqlclient_release()
{
	DE_PRINTF(1, "%s()%d : sqlite close \"%s\"\n",
				__FUNCTION__, __LINE__, get_global_conf()->db_name);
	sqlite3_close(sqlite_db);
}

int sqlclient_add_zdev(gw_info_t *p_gw, dev_info_t *m_dev)
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

	if(sqlclient_query_zdev(p_gw, m_dev->zidentity_no) == 0)
	{
		SET_CMD_LINE("%s%04X%s%s%s%d%s%s%s%s%s%s%s%s%s%s%s%s%s", 
				"UPDATE devices SET shortaddr=\'",
				m_dev->znet_addr,
				"\', apptype=\'",
				get_frapp_type_to_str(m_dev->zapp_type),
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

		return sqlclient_excute_cmdline(GET_CMD_LINE());
	}

	SET_CMD_LINE("%s%s%s%s%s%s%s%04X%s%s%s%s%s%s%s%s%s%s%s%d%s%s%s%s%s%s%s", 
		"INSERT INTO devices (id, serialnum, apptype, shortaddr, ipaddr, ",
		"commtocol, gwsn, name, area, updatetime, isonline, iscollect, ",
		"ispublic, ischange, users, data, created_at, updated_at) VALUES (NULL, \'",
		serno,
		"\', \'",
		get_frapp_type_to_str(m_dev->zapp_type),
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

	return sqlclient_excute_cmdline(GET_CMD_LINE());
}

int sqlclient_query_zdev(gw_info_t *p_gw, zidentify_no_t zidentity_no)
{
	char serstr[24] = {0};
	incode_xtocs(serstr, zidentity_no, sizeof(zidentify_no_t));
	SET_CMD_LINE("%s%s%s", 
		"SELECT * FROM devices WHERE serialnum=\'", 
		serstr, 
		"\'");

 	sqlite3_stmt *stmt;
    if(sqlite3_prepare_v2(sqlite_db,
			GET_CMD_LINE(), -1, &stmt, NULL) != SQLITE_OK)
    {
		sqlite3_finalize(stmt);
        DE_PRINTF(1, "%s()%d : sqlite prepare failed\n", __FUNCTION__, __LINE__);
		DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__, errmsg);
        return -1;
    }

	while(sqlite3_step(stmt) == SQLITE_ROW)
	{
		int counts = sqlite3_column_count(stmt);
		if(counts > 0)
		{
			sqlite3_finalize(stmt);
			return 0;
		}
	}
	sqlite3_finalize(stmt);

	return 1;
}

int sqlclient_del_zdev(gw_info_t *p_gw, zidentify_no_t zidentity_no)
{
	char serno[24] = {0};
	incode_xtocs(serno, zidentity_no, sizeof(zidentify_no_t));

	SET_CMD_LINE("%s%s%s", "DELETE FROM devices WHERE serialnum=\'",
		serno,
		"\'");

	//DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
	return sqlclient_excute_cmdline(GET_CMD_LINE());
}

int sqlclient_uponline_zdev(gw_info_t *p_gw, 
			uint8 isonline , uint16* znet_addrs, int addrs_len)
{
	int i = 0;
	char gwno[24] = {0};
	char addrs_str[512] = {0};

	if(p_gw == NULL || addrs_len <= 0)
	{
		return -1;
	}

	sprintf(addrs_str, "\'%04X\'", znet_addrs[i++]);
	while( i < addrs_len )
	{
		sprintf(addrs_str+strlen(addrs_str), ", \'%04X\'", znet_addrs[i++]);
	}

	incode_xtocs(gwno, p_gw->gw_no, sizeof(zidentify_no_t));

	SET_CMD_LINE("%s%d%s%s%s%s%s%s", 
				"UPDATE devices SET isonline=\'",
				isonline,
				"\', ischange=\'1\' ",
				"WHERE shortaddr IN (",
				addrs_str,
				") AND gwsn=\'",
				gwno,
				"\'");

	//DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
	return sqlclient_excute_cmdline(GET_CMD_LINE());
}

int sqlclient_set_datachange_zdev(zidentify_no_t dev_no, uint8 ischange)
{
	char devno_str[24] = {0};

	if(dev_no == NULL )
	{
		return -1;
	}

	incode_xtocs(devno_str, dev_no, sizeof(zidentify_no_t));
	if(!memcmp(dev_no, get_gateway_info()->gw_no, sizeof(zidentify_no_t)))
	{
		SET_CMD_LINE("%s%d%s%s%s", 
						"UPDATE devices SET ischange=\'",
						ischange,
						"\' WHERE gwsn=\'",
						devno_str,
						"\'");
	}
	else
	{
		SET_CMD_LINE("%s%d%s%s%s", 
						"UPDATE devices SET ischange=\'",
						ischange,
						"\' WHERE serialnum=\'",
						devno_str,
						"\'");
	}

	//DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
	return sqlclient_excute_cmdline(GET_CMD_LINE());
}

int sqlclient_add_gateway(gw_info_t *m_gw)
{
	char gwno_str[24] = {0};
	incode_xtocs(gwno_str, m_gw->gw_no, sizeof(zidentify_no_t));

	fr_buffer_t *frbuffer = get_devopt_data_tostr(m_gw->zgw_opt);
	char data[24] = {0};
	if(frbuffer != NULL)
	{
		memcpy(data, frbuffer->data, frbuffer->size);
		get_buffer_free(frbuffer);
	}

	int ret = sqlclient_query_gateway(m_gw->gw_no);
	if(ret > 0)
	{
		SET_CMD_LINE("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", 
			"INSERT INTO gateways (id, gwsn, apptype, ipaddr, ",
			"name, data, created_at, updated_at) VALUES (NULL, \'",
			gwno_str,
			"\', \'",
			get_frapp_type_to_str(m_gw->zapp_type),
			"\', \'",
			m_gw->ipaddr,
			"\', \'",
			get_mix_name(m_gw->zapp_type, 
				m_gw->gw_no[sizeof(zidentify_no_t)-2], 
				m_gw->gw_no[sizeof(zidentify_no_t)-1]),
			"\', \'",
			data,
			"\', \'",
			get_current_time(),
			"\', \'",
			get_current_time(),
			"\')");

		DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
		return sqlclient_excute_cmdline(GET_CMD_LINE());
	}
	else if(ret == 0)
	{
		SET_CMD_LINE("%s%s%s%s%s%s%s%s%s%s%s", 
				"UPDATE gateways SET apptype=\'",
				get_frapp_type_to_str(m_gw->zapp_type),
				"\', data=\'",
				data,
				"\', ipaddr=\'",
				m_gw->ipaddr,
				"\', updated_at=\'",
				get_current_time(),
				"\' WHERE gwsn=\'",
				gwno_str,
				"\'");

		DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
		return sqlclient_excute_cmdline(GET_CMD_LINE());
	}

	return -1;
}

int sqlclient_query_gateway(zidentify_no_t gw_no)
{
	char gwno_str[24] = {0};
	incode_xtocs(gwno_str, gw_no, sizeof(zidentify_no_t));
	SET_CMD_LINE("%s%s%s", 
		"SELECT * FROM gateways WHERE gwsn=\'", 
		gwno_str, 
		"\'");

	//DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());

 	sqlite3_stmt *stmt;
    if(sqlite3_prepare_v2(sqlite_db,
			GET_CMD_LINE(), -1, &stmt, NULL) != SQLITE_OK)
    {
		sqlite3_finalize(stmt);
        DE_PRINTF(1, "%s()%d : sqlite prepare failed\n", __FUNCTION__, __LINE__);
		DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__, errmsg);
        return -1;
    }

	while(sqlite3_step(stmt) == SQLITE_ROW)
	{
		int counts = sqlite3_column_count(stmt);
		if(counts > 0)
		{
			sqlite3_finalize(stmt);
			return 0;
		}
	}
	sqlite3_finalize(stmt);

	return 1;
}

int sqlclient_del_gateway(zidentify_no_t gw_no)
{
	char gwno_str[24] = {0};
	incode_xtocs(gwno_str, gw_no, sizeof(zidentify_no_t));

	SET_CMD_LINE("%s%s%s", "DELETE FROM devices WHERE serialnum=\'",
		gwno_str,
		"\'");

	//DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
	return sqlclient_excute_cmdline(GET_CMD_LINE());
}

void sqlclient_get_zdevices(uint8 isrefresh, trfield_device_t ***devices, int *dev_size)
{
	if(devices == NULL || dev_size == NULL)
	{
		return;
	}

	sn_t gwsn = {0};
	incode_xtocs(gwsn, get_gateway_info()->gw_no, sizeof(zidentify_no_t));

	if(isrefresh)
	{
		SET_CMD_LINE("%s%s%s",
						"SELECT * FROM devices WHERE gwsn=\'",
						gwsn,
						"\'");
	}
	else
	{
		SET_CMD_LINE("%s%s%s",
						"SELECT * FROM devices WHERE gwsn=\'",
						gwsn,
						"\' AND ischange=\'1\'");
	}

 	sqlite3_stmt *stmt;
    if(sqlite3_prepare_v2(sqlite_db, GET_CMD_LINE(), -1, &stmt, NULL) == SQLITE_OK)
    {
        while(sqlite3_step(stmt) == SQLITE_ROW)
		{
			if(sqlite3_column_count(stmt) > 17)
			{
				const uint8 *name = sqlite3_column_text(stmt, 7);
				const uint8 *dev_sn = sqlite3_column_text(stmt, 1);
				const uint8 *dev_type = sqlite3_column_text(stmt, 2);
				int isonline = sqlite3_column_int(stmt, 10);
				const uint8 *dev_data = sqlite3_column_text(stmt, 15);

				trfield_device_t *device = 
					get_trfield_device_alloc((char *)name,
												(char *)dev_sn,
												(char *)dev_type,
												isonline,
												(char *)dev_data);
				if(device != NULL)
				{
					if(*devices == NULL)
					{
						*devices = (trfield_device_t **)calloc(1, sizeof(trfield_device_t *));
						*(*devices) = device;
					}
					else
					{
						*devices = (trfield_device_t **)realloc(*devices, (*dev_size+1)*sizeof(trfield_device_t *));
						*(*devices+*dev_size) = device;
					}

					(*dev_size)++;
				}
			}
		}
    }
	sqlite3_finalize(stmt);

	if(!isrefresh)
	{
		SET_CMD_LINE("%s%s%s",
						"UPDATE devices SET ischange=\'0\' WHERE gwsn=\'",
						gwsn,
						"\'");
		sqlclient_excute_cmdline(GET_CMD_LINE());
	}
}

void sqlclient_get_devdatas(char **text, long *text_len)
{
	if(text == NULL || text_len == NULL)
	{
		return;
	}

	sn_t gwsn = {0};
	incode_xtocs(gwsn, get_gateway_info()->gw_no, sizeof(zidentify_no_t));
	SET_CMD_LINE("%s%s%s",
					"SELECT data,isonline FROM devices WHERE gwsn=\'",
					gwsn,
					"\' ORDER BY serialnum ASC");

 	sqlite3_stmt *stmt;
    if(sqlite3_prepare_v2(sqlite_db, GET_CMD_LINE(), -1, &stmt, NULL) == SQLITE_OK)
    {
        while(sqlite3_step(stmt) == SQLITE_ROW)
		{
			if(sqlite3_column_count(stmt) > 1)
			{
				const uint8 *data = sqlite3_column_text(stmt, 0);
				const uint8 *isonline = sqlite3_column_text(stmt, 1);
				if(data == NULL || isonline == NULL)
				{
					continue;
				}

				int data_len = strlen((char *)data);

				if(*text == NULL)
				{
					*text = (char *)calloc(1, data_len+2);
					*text_len = data_len + 2;
				}
				else
				{
					*text = (char *)realloc(*text, *text_len+data_len+1);
					*text_len += data_len+1;
				}

				memcpy(*text+*text_len-data_len-2, data, data_len);
				*(*text+*text_len-2) = *isonline;
				*(*text+*text_len-1) = '\0';
			}
		}
    }
	sqlite3_finalize(stmt);
}
#endif

#ifdef __cplusplus
}
#endif

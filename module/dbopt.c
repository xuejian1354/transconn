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
#include <cJSON.h>

#define DB_SERVER	"localhost"
#define DB_PORT		3306

#define NO_AREA		"未设置"

#define GET_CMD_LINE()	cmdline

#define SET_CMD_LINE(format, args...)  	\
st(  									\
	bzero(cmdline, sizeof(cmdline));  	\
	sprintf(cmdline, format, ##args);  	\
)

static MYSQL mysql_conn;
static MYSQL_RES *mysql_res;
static MYSQL_ROW mysql_row;

static pthread_mutex_t sql_lock;
static pthread_mutex_t sql_add_lock;

static char is_userful = 0;
static char cmdline[4096];
static char mix_type_name[24];
static char current_time[64];

pthread_mutex_t *get_sql_add_lock()
{
	return &sql_add_lock;
}

char *get_mix_name(fr_app_type_t type, uint8 s1, uint8 s2)
{
	bzero(mix_type_name, sizeof(mix_type_name));
	sprintf(mix_type_name, "%s%02X%02X", get_name_from_type(type), s1, s2);
	return mix_type_name;
}

char *get_current_time()
{
	time_t t;
	time(&t);
	bzero(current_time, sizeof(current_time));
	struct tm *tp= localtime(&t);
	strftime(current_time, 100, "%Y-%m-%d %H:%M:%S", tp); 

	return current_time;
}

int sql_init()
{
	global_conf_t *m_conf = get_global_conf();

	if (mysql_init(&mysql_conn) == NULL)
	{
		printf("%s()%d: sql init failed\n", __FUNCTION__, __LINE__);
		return -1;
	}

	is_userful = 1;

	mysql_options(&mysql_conn, MYSQL_OPT_RECONNECT, &is_userful);

	if (mysql_real_connect(&mysql_conn, DB_SERVER, m_conf->db_user, 
		m_conf->db_password, m_conf->db_name, 0, NULL, 0) == NULL)
	{
		printf("%s()%d : sql connect \"%s\" failed\n", 
			__FUNCTION__, __LINE__, m_conf->db_name);
		return -1;
	}

	printf("sql connect \"%s\"\n", m_conf->db_name);

	if(mysql_set_character_set(&mysql_conn, "utf8"))
	{
		printf("%s()%d : sql character set failed\n", __FUNCTION__, __LINE__);
		return -1;
	} 

	if(pthread_mutex_init(&sql_lock, NULL) != 0)
	{
		printf("%s()%d :  pthread_mutext_init failed\n", __FUNCTION__, __LINE__);
        return -1;
	}

	if(pthread_mutex_init(&sql_add_lock, NULL) != 0)
	{
		fprintf(stderr, "%s()%d :  pthread_mutext_init failed\n",
			__FUNCTION__, __LINE__);
		return -1;
	}

	return 0;
}

int sql_reconnect()
{
	global_conf_t *m_conf = get_global_conf();
	
	if(mysql_ping(&mysql_conn))
	{
		is_userful = 0;
		if (mysql_real_connect(&mysql_conn, DB_SERVER, m_conf->db_user, 
			m_conf->db_password, m_conf->db_name, 0, NULL, 0) == NULL)
		{
			DE_PRINTF(1, "%s()%d : sql connect \"%s\" failed\n", 
				__FUNCTION__, __LINE__, m_conf->db_name);
			return -1;
		}

		DE_PRINTF(1, "%s()%d : sql connect \"%s\"\n", 
			__FUNCTION__, __LINE__, m_conf->db_name);

		if(mysql_set_character_set(&mysql_conn, "utf8"))
		{
			DE_PRINTF(1, "%s()%d : sql character set failed\n", 
					__FUNCTION__, __LINE__);
			return -1;
		}

		is_userful = 1;
		return 1;
	}

	return 0;
}

void sql_release()
{
	is_userful = 0;
	DE_PRINTF(1, "%s()%d : sql close \"%s\"\n", 
	 			__FUNCTION__, __LINE__, get_global_conf()->db_name);	
	mysql_close(&mysql_conn);
}

int sql_isuse()
{
	return is_userful;
}

int sql_add_zdev(gw_info_t *p_gw, dev_info_t *m_dev)
{
	char apptype_str[4] = {0};
	get_frapp_type_to_str(apptype_str, m_dev->zapp_type);

	char serno[24] = {0};
	incode_xtocs(serno, m_dev->zidentity_no, sizeof(zidentify_no_t));

	char gwno[24] = {0};
	incode_xtocs(gwno, p_gw->gw_no, sizeof(zidentify_no_t));
	
	fr_buffer_t *frbuffer = get_devopt_data_to_str(m_dev->zdev_opt);
	char data[24] = {0};
	if(frbuffer != NULL)
	{
		memcpy(data, frbuffer->data, frbuffer->size);
		get_buffer_free(frbuffer);
	}

	if(sql_query_zdev(p_gw, m_dev->zidentity_no) != NULL)
	{
		SET_CMD_LINE("%s%04X%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", 
				"UPDATE devices SET shortaddr=\'",
				m_dev->znet_addr,
				"\', apptype=\'",
				apptype_str,
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
		pthread_mutex_lock(&sql_lock);
		if(sql_reconnect() < 0)
		{
			pthread_mutex_unlock(&sql_lock);
		   	return -1;
		}
		
		if( mysql_query(&mysql_conn, GET_CMD_LINE()))
	    {
			pthread_mutex_unlock(&sql_lock);
	       	DE_PRINTF(1, "%s()%d : sql query devices failed\n\n", __FUNCTION__, __LINE__);
		   	return -1;
	    }
		pthread_mutex_unlock(&sql_lock);

		return add_zdev_info(p_gw, m_dev);
	}

	SET_CMD_LINE("%s%s%s%s%s%s%s%04X%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", 
		"INSERT INTO devices (id, serialnum, apptype, shortaddr, ipaddr, ",
		"commtocol, gwsn, name, area, updatetime, isonline, iscollect, ",
		"ispublic, users, data, created_at, updated_at) VALUES (NULL, \'",
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
		"\', \'1\', \'0\', \'0\', \'\', \'",
		data,
		"\', \'",
		get_current_time(),
		"\', \'",
		get_current_time(),
		"\')");

	DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
	pthread_mutex_lock(&sql_lock);
	if(sql_reconnect() < 0)
	{
		pthread_mutex_unlock(&sql_lock);
	   	return -1;
	}
	
	if( mysql_query(&mysql_conn, GET_CMD_LINE()))
    {
		pthread_mutex_unlock(&sql_lock);
       	DE_PRINTF(1, "%s()%d : sql query devices failed\n\n", __FUNCTION__, __LINE__);
	   	return -1;
    }
	pthread_mutex_unlock(&sql_lock);
	
	return add_zdev_info(p_gw, m_dev);
}

dev_info_t *sql_query_zdev(gw_info_t *p_gw, zidentify_no_t zidentity_no)
{
	char serstr[24] = {0};
	incode_xtocs(serstr, zidentity_no, sizeof(zidentify_no_t));
	SET_CMD_LINE("%s%s%s", 
		"SELECT * FROM devices WHERE serialnum=\'", 
		serstr, 
		"\'");

	//DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
	pthread_mutex_lock(&sql_lock);
	if(sql_reconnect() < 0)
	{
		pthread_mutex_unlock(&sql_lock);
	   	return NULL;
	}
	
	if( mysql_query(&mysql_conn, GET_CMD_LINE()))
    {
		pthread_mutex_unlock(&sql_lock);
       	DE_PRINTF(1, "%s()%d : sql query devices failed\n\n", __FUNCTION__, __LINE__);
	   	return NULL;
    }
	pthread_mutex_unlock(&sql_lock);

	if((mysql_res = mysql_store_result(&mysql_conn)) == NULL)
	{
		DE_PRINTF(1, "%s()%d : sql store result failed\n\n", __FUNCTION__, __LINE__);
		return NULL;
	}

    while((mysql_row = mysql_fetch_row(mysql_res)))
    {
		int nums = mysql_num_fields(mysql_res);
		uint16 znet_addr;
		incode_ctox16(&znet_addr, mysql_row[3]);
		dev_info_t *m_dev = calloc(1, sizeof(dev_info_t));

		memcpy(m_dev->zidentity_no, zidentity_no, sizeof(zidentify_no_t));
		m_dev->znet_addr = znet_addr;
		m_dev->zapp_type = get_frapp_type_from_str(mysql_row[2]);
		m_dev->znet_type = FRNET_ENDDEV;

		m_dev->zdev_opt = calloc(1, sizeof(dev_opt_t));
		m_dev->zdev_opt->type = m_dev->zapp_type;
		if(set_devopt_fromstr(m_dev->zdev_opt, mysql_row[14], strlen(mysql_row[14])) < 0)
		{
			free(m_dev->zdev_opt);
			m_dev->zdev_opt = NULL;
		}

		if(add_zdev_info(p_gw, m_dev) != 0)
		{
			get_devopt_data_free(m_dev->zdev_opt);
			get_zdev_frame_free(m_dev);
		}
		
		mysql_free_result(mysql_res);
		return query_zdev_info(p_gw, znet_addr);
	}

    mysql_free_result(mysql_res);
	return NULL;
}

int sql_del_zdev(gw_info_t *p_gw, zidentify_no_t zidentity_no)
{
	char serno[24] = {0};
	incode_xtocs(serno, zidentity_no, sizeof(zidentify_no_t));
	
	SET_CMD_LINE("%s%s%s", "DELETE FROM devices WHERE serialnum=\'",
		serno,
		"\'");

	//DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
	pthread_mutex_lock(&sql_lock);
	if(sql_reconnect() < 0)
	{
		pthread_mutex_unlock(&sql_lock);
	   	return -1;
	}
	
	if( mysql_query(&mysql_conn, GET_CMD_LINE()))
    {
		pthread_mutex_unlock(&sql_lock);
       	DE_PRINTF(1, "%s()%d : sql query devices failed\n\n", __FUNCTION__, __LINE__);
	   	return -1;
    }
	pthread_mutex_unlock(&sql_lock);
	
	return 0;
}

int sql_uponline_zdev(gw_info_t *p_gw, 
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

	SET_CMD_LINE("%s%d%s%s%s%s%s", 
				"UPDATE devices SET isonline=\'",
				isonline,
				"\' WHERE shortaddr IN (",
				addrs_str,
				") AND gwsn=\'",
				gwno,
				"\'");

	//DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
	pthread_mutex_lock(&sql_lock);
	if(sql_reconnect() < 0)
	{
		pthread_mutex_unlock(&sql_lock);
	   	return -1;
	}
	
	if( mysql_query(&mysql_conn, GET_CMD_LINE()))
    {
		pthread_mutex_unlock(&sql_lock);
       	DE_PRINTF(1, "%s()%d : sql query devices failed\n\n", __FUNCTION__, __LINE__);
	   	return -1;
    }
	pthread_mutex_unlock(&sql_lock);
	
	return 0;
}

int sql_add_gateway(gw_info_t *m_gw)
{
	char apptype_str[4] = {0};
	get_frapp_type_to_str(apptype_str, m_gw->zapp_type);

	char gwno_str[24] = {0};
	incode_xtocs(gwno_str, m_gw->gw_no, sizeof(zidentify_no_t));
	
	fr_buffer_t *frbuffer = get_devopt_data_to_str(m_gw->zgw_opt);
	char data[24] = {0};
	if(frbuffer != NULL)
	{
		memcpy(data, frbuffer->data, frbuffer->size);
		get_buffer_free(frbuffer);
	}

	int ret = sql_query_gateway(m_gw->gw_no);
	if(ret > 0)
	{
		SET_CMD_LINE("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", 
			"INSERT INTO gateways (id, gwsn, apptype, ipaddr, ",
			"name, data, created_at, updated_at) VALUES (NULL, \'",
			gwno_str,
			"\', \'",
			apptype_str,
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
		pthread_mutex_lock(&sql_lock);
		if(sql_reconnect() < 0)
		{
			pthread_mutex_unlock(&sql_lock);
		   	return -1;
		}
		
		if( mysql_query(&mysql_conn, GET_CMD_LINE()))
	    {
			pthread_mutex_unlock(&sql_lock);
	       	DE_PRINTF(1, "%s()%d : sql query devices failed\n\n", __FUNCTION__, __LINE__);
		   	return -1;
	    }
		pthread_mutex_unlock(&sql_lock);
		
		return add_gateway_info(m_gw);

	}
	else if(ret == 0)
	{
		SET_CMD_LINE("%s%s%s%s%s%s%s%s%s%s%s", 
				"UPDATE gateways SET apptype=\'",
				apptype_str,
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
		pthread_mutex_lock(&sql_lock);
		if(sql_reconnect() < 0)
		{
			pthread_mutex_unlock(&sql_lock);
		   	return -1;
		}
		
		if( mysql_query(&mysql_conn, GET_CMD_LINE()))
	    {
			pthread_mutex_unlock(&sql_lock);
	       	DE_PRINTF(1, "%s()%d : sql query devices failed\n\n", __FUNCTION__, __LINE__);
		   	return -1;
	    }
		pthread_mutex_unlock(&sql_lock);

		return add_gateway_info(m_gw);
	}

	return -1;
}

int sql_query_gateway(zidentify_no_t gw_no)
{
	char gwno_str[24] = {0};
	incode_xtocs(gwno_str, gw_no, sizeof(zidentify_no_t));
	SET_CMD_LINE("%s%s%s", 
		"SELECT * FROM gateways WHERE gwsn=\'", 
		gwno_str, 
		"\'");

	//DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
	pthread_mutex_lock(&sql_lock);
	if(sql_reconnect() < 0)
	{
		pthread_mutex_unlock(&sql_lock);
	   	return -1;
	}
	
	if( mysql_query(&mysql_conn, GET_CMD_LINE()))
    {
		pthread_mutex_unlock(&sql_lock);
       	DE_PRINTF(1, "%s()%d : sql query devices failed\n\n", __FUNCTION__, __LINE__);
	   	return -1;
    }
	pthread_mutex_unlock(&sql_lock);

	if((mysql_res = mysql_store_result(&mysql_conn)) == NULL)
	{
		DE_PRINTF(1, "%s()%d : sql store result failed\n\n", __FUNCTION__, __LINE__);
		return -1;
	}

    while((mysql_row = mysql_fetch_row(mysql_res)))
    {
		int nums = mysql_num_fields(mysql_res);
		if(nums > 1 && !memcmp(mysql_row[1], gwno_str, sizeof(zidentify_no_t)))
		{
			mysql_free_result(mysql_res);
			return 0;
		}
	}

    mysql_free_result(mysql_res);
	return 1;
}

int sql_del_gateway(zidentify_no_t gw_no)
{
	char gwno_str[24] = {0};
	incode_xtocs(gwno_str, gw_no, sizeof(zidentify_no_t));
	
	SET_CMD_LINE("%s%s%s", "DELETE FROM devices WHERE serialnum=\'",
		gwno_str,
		"\'");

	//DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
	pthread_mutex_lock(&sql_lock);
	if(sql_reconnect() < 0)
	{
		pthread_mutex_unlock(&sql_lock);
	   	return -1;
	}
	
	if( mysql_query(&mysql_conn, GET_CMD_LINE()))
    {
		pthread_mutex_unlock(&sql_lock);
       	DE_PRINTF(1, "%s()%d : sql query devices failed\n\n", __FUNCTION__, __LINE__);
	   	return -1;
    }
	pthread_mutex_unlock(&sql_lock);
	
	return 0;
}

int set_user_info_from_sql(char *email, cli_user_t *user_info)
{
	if(email == NULL || user_info == NULL)
	{
		return -1;
	}

	SET_CMD_LINE("%s%s%s%s", 
		"SELECT name, devices, areas, scenes ",
		"FROM users WHERE email=\'", 
		email, 
		"\'");

	DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
	pthread_mutex_lock(&sql_lock);
	if(sql_reconnect() < 0)
	{
		pthread_mutex_unlock(&sql_lock);
	   	return -1;
	}
	
	if( mysql_query(&mysql_conn, GET_CMD_LINE()))
    {
		pthread_mutex_unlock(&sql_lock);
       	DE_PRINTF(1, "%s()%d : sql query devices failed\n\n", __FUNCTION__, __LINE__);
	   	return -1;
    }
	pthread_mutex_unlock(&sql_lock);

	if((mysql_res = mysql_store_result(&mysql_conn)) == NULL)
	{
		DE_PRINTF(1, "%s()%d : sql store result failed\n\n", __FUNCTION__, __LINE__);
		return -1;
	}

    while((mysql_row = mysql_fetch_row(mysql_res)))
    {
		int nums = mysql_num_fields(mysql_res);
		if(nums >= 4)
		{
			int name_len = strlen(mysql_row[0]);
			memcpy(user_info->name, "name:", 5);
			memcpy(user_info->name+5, mysql_row[0], 
				sizeof(user_info->name)-6<name_len?sizeof(user_info->name)-6:name_len);

			if(user_info->devices != NULL)
			{
				free(user_info->devices);
				user_info->devices = NULL;
			}
			
			if(mysql_row[1] != NULL && strlen(mysql_row[1]) > 0)
			{
				user_info->devices = calloc(1, strlen(mysql_row[1])+9);
				memcpy(user_info->devices, "devices:", 8);
				memcpy(user_info->devices+8, mysql_row[1], strlen(mysql_row[1]));
			}
			
			if(user_info->areas != NULL)
			{
				free(user_info->areas);
				user_info->areas = NULL;
			}

			if(mysql_row[2] != NULL && strlen(mysql_row[2]) > 0)
			{
				user_info->areas = calloc(1, strlen(mysql_row[2])+7);
				memcpy(user_info->areas, "areas:", 6);
				memcpy(user_info->areas+6, mysql_row[2], strlen(mysql_row[2]));
			}
			
			if(user_info->scenes != NULL)
			{
				free(user_info->scenes);
				user_info->scenes = NULL;
			}

			if(mysql_row[3] != NULL && strlen(mysql_row[3]) > 0)
			{
				user_info->scenes = calloc(1, strlen(mysql_row[3])+8);
				memcpy(user_info->scenes, "scenes:", 7);
				memcpy(user_info->scenes+7, mysql_row[3], strlen(mysql_row[3]));
			}
			
			mysql_free_result(mysql_res);
			return 0;
		}
	}

    mysql_free_result(mysql_res);
	return 1;	
}

int set_zdev_to_user_sql(char *email, char *dev_str)
{
	if(email == NULL)
	{
		return -1;
	}
	
	SET_CMD_LINE("%s%s%s", 
		"SELECT devices FROM users WHERE email=\'", 
		email, 
		"\'");

	//DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
	pthread_mutex_lock(&sql_lock);
	if(sql_reconnect() < 0)
	{
		pthread_mutex_unlock(&sql_lock);
	   	return -1;
	}
	
	if( mysql_query(&mysql_conn, GET_CMD_LINE()))
    {
		pthread_mutex_unlock(&sql_lock);
       	DE_PRINTF(1, "%s()%d : sql query devices failed\n\n", __FUNCTION__, __LINE__);
	   	return -1;
    }
	pthread_mutex_unlock(&sql_lock);

	if((mysql_res = mysql_store_result(&mysql_conn)) == NULL)
	{
		DE_PRINTF(1, "%s()%d : sql store result failed\n\n", __FUNCTION__, __LINE__);
		return -1;
	}

    while((mysql_row = mysql_fetch_row(mysql_res)))
    {
		int nums = mysql_num_fields(mysql_res);
		if(nums > 0)
		{
			cJSON *pRoot = cJSON_Parse(mysql_row[0]);
			if(pRoot == NULL)
			{
				continue;
			}

			cJSON* pDeviceArray = cJSON_GetObjectItem(pRoot, "devices");
			if (pDeviceArray == NULL)
			{
				continue;
			}
			 
			int dev_size = cJSON_GetArraySize(pDeviceArray);
			int i = 0; 
			while(i < dev_size)
			{
				cJSON *pDev = cJSON_GetArrayItem(pDeviceArray, i);
				char *device = cJSON_GetObjectItem(pDev, "sn")->valuestring;
				if(device != NULL && !memcmp(dev_str, device, 16))
				{
					cJSON_Delete(pRoot);
					mysql_free_result(mysql_res);
					return 1;
				}
				
				i++;
			}
			
			cJSON *pDev = cJSON_CreateObject();
			cJSON_AddItemToArray(pDeviceArray, pDev);
			
			cJSON_AddStringToObject(pDev, "sn", dev_str);
			cJSON_AddStringToObject(pDev, "iscollect", "0");
			cJSON_AddStringToObject(pDev, "locate", "");

			char *devices = cJSON_Print(pRoot);

			SET_CMD_LINE("%s%s%s%s%s", 
						"UPDATE users SET devices=\'",
						devices,
						"\' WHERE email=\'",
						email,
						"\'");

			DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
			pthread_mutex_lock(&sql_lock);
			if(sql_reconnect() < 0)
			{
				pthread_mutex_unlock(&sql_lock);
				
				cJSON_Delete(pRoot);
				free(devices);
				
				mysql_free_result(mysql_res);
				
			   	return -1;
			}
			
			if( mysql_query(&mysql_conn, GET_CMD_LINE()))
		    {
				pthread_mutex_unlock(&sql_lock);
				
				cJSON_Delete(pRoot);
				free(devices);
				
				mysql_free_result(mysql_res);

				DE_PRINTF(1, "%s()%d : sql query devices failed\n\n", 
						__FUNCTION__, __LINE__);
				
			   	return -1;
		    }
			pthread_mutex_unlock(&sql_lock);
			
			cJSON_Delete(pRoot);
			free(devices);
			
			mysql_free_result(mysql_res);
			return 0;
		}
	}

	cJSON *pRoot = cJSON_CreateObject();
	
	cJSON *pDevsArray = cJSON_CreateArray();
	cJSON_AddItemToObject(pRoot, "devices", pDevsArray);
	
	cJSON *pDev = cJSON_CreateObject();
	cJSON_AddItemToArray(pDevsArray, pDev);
	
	cJSON_AddStringToObject(pDev, "sn", dev_str);
	cJSON_AddStringToObject(pDev, "iscollect", "0");
	cJSON_AddStringToObject(pDev, "locate", "");

	char *devices = cJSON_Print(pRoot);

	SET_CMD_LINE("%s%s%s%s%s", 
				"UPDATE users SET devices=\'",
				devices,
				"\' WHERE email=\'",
				email,
				"\'");

	DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
	pthread_mutex_lock(&sql_lock);
	if(sql_reconnect() < 0)
	{
		pthread_mutex_unlock(&sql_lock);
		
		cJSON_Delete(pRoot);
		free(devices);
		
		mysql_free_result(mysql_res);
		
	   	return -1;
	}
	
	if( mysql_query(&mysql_conn, GET_CMD_LINE()))
    {
		pthread_mutex_unlock(&sql_lock);
		
		cJSON_Delete(pRoot);
		free(devices);
		
		mysql_free_result(mysql_res);

		DE_PRINTF(1, "%s()%d : sql query devices failed\n\n", 
				__FUNCTION__, __LINE__);
		
	   	return -1;
    }
	pthread_mutex_unlock(&sql_lock);
	
	cJSON_Delete(pRoot);
	free(devices);

    mysql_free_result(mysql_res);
	return 1;
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
		 	printf("%s()%d : sql connection failed.\n", __FUNCTION__, __LINE__);
		 }
      }
	 else
	 {
	 	printf("%s()%d : sql initialization failed.\n", __FUNCTION__, __LINE__);
	 }

     mysql_close(&mysql_conn);
}


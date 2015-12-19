/*
 * dbserver.c
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
#include "dbserver.h"
#include <module/dballoc.h>
#include <cJSON.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DB_API_WITH_MYSQL

#define DB_SERVER	"localhost"
#define DB_PORT		3306

extern char cmdline[CMDLINE_SIZE];

static MYSQL mysql_conn;
static MYSQL_RES *mysql_res;
static MYSQL_ROW mysql_row;

static char tcolumn[128];

static pthread_mutex_t sqlserver_lock;

static char is_userful = 0;

static int sqlserver_excute_cmdline(char *cmdline);

static void sync_devices_with_user_sql(char *email, devices_t *devs);
static void sync_areas_with_user_sql(char *email, areas_t *areas);
static void sync_scenes_with_user_sql(char *email, scenes_t *scenes);

static void del_devices_from_user_sql(char *email, devices_t *devs);
static void del_areas_from_user_sql(char *email, areas_t *areas);
static void del_scenes_from_user_sql(char *email, scenes_t *scenes);

int sqlserver_init()
{
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

	if(pthread_mutex_init(&sqlserver_lock, NULL) != 0)
	{
		DE_PRINTF(1, "%s()%d : pthread_mutext_init failed\n", __FUNCTION__, __LINE__);
        return -1;
	}

	return 0;
}

int sqlserver_reconnect()
{
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

	return 0;
}

 int sqlserver_excute_cmdline(char *cmdline)
 {
	pthread_mutex_lock(&sqlserver_lock);
	if(sqlserver_reconnect() < 0)
	{
		pthread_mutex_unlock(&sqlserver_lock);
	   	return -1;
	}

	if( mysql_query(&mysql_conn, cmdline))
    {
		pthread_mutex_unlock(&sqlserver_lock);
       	DE_PRINTF(1, "%s()%d : mysql query failed\n\n", __FUNCTION__, __LINE__);
	   	return -1;
    }

	pthread_mutex_unlock(&sqlserver_lock);
	return 0;
 }

void sqlserver_release()
{
	is_userful = 0;
	DE_PRINTF(1, "%s()%d : mysql close \"%s\"\n",
	 			__FUNCTION__, __LINE__, get_global_conf()->db_name);
	mysql_close(&mysql_conn);
}

int sqlserver_isuse()
{
	return is_userful;
}

int sqlserver_add_gateway(frhandler_arg_t *arg, sn_t gwsn)
{
	if(arg == NULL || gwsn == NULL)
	{
		return -1;
	}

	int udp_port = 0;
	int tcp_port = 0;

	switch(arg->transtocol)
	{
	case TOCOL_UDP:
		udp_port = ntohs(arg->addr.sin_port);
		break;

	case TOCOL_TCP:
		tcp_port = ntohs(arg->addr.sin_port);
		break;
	}

	int ret = sqlserver_query_gateway(gwsn);
	if(ret == 0)
	{
		SET_CMD_LINE("%s%s%s%s%s%d%s%d%s%s%s%s%s",
				"UPDATE gateways SET transtocol=\'",
				get_trans_protocol_to_str(arg->transtocol),
				"\', ip=\'",
				inet_ntoa(arg->addr.sin_addr),
				"\', udp_port=\'",
				udp_port,
				"\', tcp_port=\'",
				tcp_port,
				"\', http_url=\'0\', updatetime=\'",
				get_current_time(),
				"\' WHERE gw_sn=\'",
				gwsn,
				"\'");

		//DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
		return sqlserver_excute_cmdline(GET_CMD_LINE());
	}
	else if(ret > 0)
	{
		uint8 rannum[2];
		incode_ctoxs(rannum, gwsn+12, 2);
		incode_ctoxs(rannum+1, gwsn+14, 2);

		SET_CMD_LINE("%s%s%s%s%s%s%s%s%s%s%s%d%s%d%s%s%s%s%s", 
			"INSERT INTO gateways (id, name, gw_sn, transtocol, ",
			"ip, udp_port, tcp_port, http_url, area, updatetime) ",
			"VALUES (NULL, \'",
			get_mix_name(FRAPP_CONNECTOR, rannum[0], rannum[1]),
			"\', \'",
			gwsn,
			"\', \'",
			get_trans_protocol_to_str(arg->transtocol),
			"\', \'",
			inet_ntoa(arg->addr.sin_addr),
			"\', \'",
			udp_port,
			"\', \'",
			tcp_port,
			"\', \'0\', \'",
			NO_AREA,
			"\', \'",
			get_current_time(),
			"\')");

		//DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
		return sqlserver_excute_cmdline(GET_CMD_LINE());
	}

	return -1;
}

int sqlserver_query_gateway(sn_t gwsn)
{
	SET_CMD_LINE("%s%s%s", 
		"SELECT * FROM gateways WHERE gw_sn=\'", 
		gwsn, 
		"\'");

	//DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
	if(sqlserver_excute_cmdline(GET_CMD_LINE()) < 0)
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

	return 1;
}

char *sqlserver_get_column_from_gwsn(char *field, sn_t gwsn)
{
	SET_CMD_LINE("%s%s%s%s%s",
		"SELECT ",
		field,
		" FROM gateways WHERE gw_sn=\'",
		gwsn, 
		"\'");

	//DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
	if(sqlserver_excute_cmdline(GET_CMD_LINE()) < 0)
	{
		return NULL;
	}

	if((mysql_res = mysql_store_result(&mysql_conn)) == NULL)
	{
		DE_PRINTF(1, "%s()%d : mysql store result failed\n\n", __FUNCTION__, __LINE__);
		return NULL;
	}

    while((mysql_row = mysql_fetch_row(mysql_res)))
    {
		int nums = mysql_num_fields(mysql_res);
		if(nums > 0)
		{
			STRS_MEMCPY(tcolumn, mysql_row[0], sizeof(tcolumn), strlen(mysql_row[0]));
			mysql_free_result(mysql_res);
			return tcolumn;
		}
	}
	mysql_free_result(mysql_res);

	return NULL;
}

int sqlserver_add_zdevices(frhandler_arg_t *arg, trfr_report_t *report)
{
	if(arg == NULL || report == NULL
		|| report->devices == NULL || report->dev_size <= 0)
	{
		return -1;
	}

	int i;
	for(i=0; i<report->dev_size; i++)
	{
		trfield_device_t *device = *(report->devices+i);
		int ret = sqlserver_query_zdevice(device->dev_sn);
		if(ret == 0)
		{
			SET_CMD_LINE("%s%s%s%d%s%s%s%s%s%s%s%s%s", 
					"UPDATE devices SET dev_type=\'",
					get_frapp_type_to_str(device->dev_type),
					"\', znet_status=\'",
					device->znet_status,
					"\', dev_data=\'",
					device->dev_data,
					"\', gw_sn=\'",
					report->gw_sn,
					"\', updatetime=\'",
					get_current_time(),
					"\' WHERE dev_sn=\'",
					device->dev_sn,
					"\'");

			DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
			sqlserver_excute_cmdline(GET_CMD_LINE());
		}
		else if(ret > 0)
		{
			uint8 rannum[2];
			incode_ctoxs(rannum, report->gw_sn+14, 2);
			incode_ctoxs(rannum+1, device->dev_sn+14, 2);

			SET_CMD_LINE("%s%s%s%s%s%s%s%s%d%s%s%s%s%s%s%s%s%s", 
				"INSERT INTO devices (id, dev_sn, name, dev_type, znet_status, ",
				"dev_data, gw_sn, area, ispublic, updatetime) VALUES (NULL, \'",
				device->dev_sn,
				"\', \'",
				get_mix_name(device->dev_type, rannum[0], rannum[1]),
				"\', \'",
				get_frapp_type_to_str(device->dev_type),
				"\', \'",
				device->znet_status,
				"\', \'",
				device->dev_data,
				"\', \'",
				report->gw_sn,
				"\', \'",
				NO_AREA,
				"\', \'0\', \'",
				get_current_time(),
				"\')");

			DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
			sqlserver_excute_cmdline(GET_CMD_LINE());
		}
	}
}

int sqlserver_update_zdevice(frhandler_arg_t *arg, trfr_respond_t *respond)
{
	if(arg == NULL || respond == NULL)
	{
		return -1;
	}

	if(sqlserver_query_zdevice(respond->dev_sn) == 0)
	{
		SET_CMD_LINE("%s%s%s%s%s%s%s", 
				"UPDATE devices SET dev_data=\'",
				respond->dev_data,
				"\', updatetime=\'",
				get_current_time(),
				"\' WHERE dev_sn=\'",
				respond->dev_sn,
				"\'");

		//DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
		return sqlserver_excute_cmdline(GET_CMD_LINE());
	}

	return 1;
}

int sqlserver_query_zdevice(sn_t serno)
{
	SET_CMD_LINE("%s%s%s", 
		"SELECT * FROM devices WHERE dev_sn=\'", 
		serno, 
		"\'");

	//DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
	if(sqlserver_excute_cmdline(GET_CMD_LINE()) < 0)
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

	return 1;
}

char *sqlserver_get_column_from_zdevice(char *field, sn_t serno)
{
	SET_CMD_LINE("%s%s%s%s%s", 
		"SELECT ",
		field,
		" FROM devices WHERE dev_sn=\'", 
		serno, 
		"\'");

	//DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
	if(sqlserver_excute_cmdline(GET_CMD_LINE()) < 0)
	{
		return NULL;
	}

	if((mysql_res = mysql_store_result(&mysql_conn)) == NULL)
	{
		DE_PRINTF(1, "%s()%d : mysql store result failed\n\n", __FUNCTION__, __LINE__);
		return NULL;
	}

    while((mysql_row = mysql_fetch_row(mysql_res)))
    {
		int nums = mysql_num_fields(mysql_res);
		if(nums > 0)
		{
			STRS_MEMCPY(tcolumn, mysql_row[0], sizeof(tcolumn), strlen(mysql_row[0]));
			mysql_free_result(mysql_res);
			return tcolumn;
		}
	}
	mysql_free_result(mysql_res);

	return NULL;
}

int sqlserver_del_zdevice(sn_t serno)
{
	SET_CMD_LINE("%s%s%s", "DELETE FROM devices WHERE dev_sn=\'",
		serno,
		"\'");

	//DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
	return sqlserver_excute_cmdline(GET_CMD_LINE());
}


int get_user_info_from_sql(char *email, cli_user_t *user_info)
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
	if(sqlserver_excute_cmdline(GET_CMD_LINE()) < 0)
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
				user_info->devices = (char *)calloc(1, strlen(mysql_row[1])+9);
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
				user_info->areas = (char *)calloc(1, strlen(mysql_row[2])+7);
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
				user_info->scenes = (char *)calloc(1, strlen(mysql_row[3])+8);
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

void sync_user_info_to_sql(char *data)
{
	char *email = NULL;
	cJSON* pDevs = NULL;
	cJSON* pAreas = NULL;
	cJSON* pScenes = NULL;

	if(data == NULL)
	{
		return;
	}

	cJSON *pRoot = cJSON_Parse(data);
	if(pRoot == NULL)
	{
		return;
	}

	cJSON *pEmail = cJSON_GetObjectItem(pRoot, "email");
	if(pEmail == NULL)
	{
		goto sync_end;
	}

	email = pEmail->valuestring;

	pDevs = cJSON_GetObjectItem(pRoot, "devices");
	if (pDevs != NULL)
	{
		int dev_size = cJSON_GetArraySize(pDevs);
		devices_t *devs = devices_alloc(dev_size);
		int i = 0;
		while(i < dev_size)
		{
			cJSON *pDev = cJSON_GetArrayItem(pDevs, i);

			char *sn = cJSON_GetObjectItem(pDev, "sn")->valuestring;
			char *iscollect = cJSON_GetObjectItem(pDevs, "iscollect")->valuestring;
			char *locate = cJSON_GetObjectItem(pDevs, "locate")->valuestring;

			devices_add(devs, sn, atoi(iscollect), locate);

			i++;
		}
		sync_devices_with_user_sql(email, devs);
		devices_free(devs);
	}

	pAreas = cJSON_GetObjectItem(pRoot, "areas");
	if (pAreas != NULL)
	{
		int area_size = cJSON_GetArraySize(pAreas);
		areas_t *areas = areas_alloc(area_size);
		int i = 0;
		while(i < area_size)
		{
			areas_add(areas, cJSON_GetArrayItem(pAreas, i)->valuestring);
			i++;
		}
		sync_areas_with_user_sql(email, areas);
		strings_free(areas);
	}

	pScenes = cJSON_GetObjectItem(pRoot, "scenes");
	if (pScenes != NULL)
	{
		int scene_size = cJSON_GetArraySize(pScenes);
		scenes_t *scenes = scenes_alloc(scene_size);
		int i = 0;
		while(i < scene_size)
		{
			cJSON *pScene = cJSON_GetArrayItem(pScenes, i);
			
			char *name = cJSON_GetObjectItem(pScene, "name")->valuestring;

			cJSON *pSDevs = cJSON_GetObjectItem(pScene, "devices");
			strings_t *pdevs_str = NULL;
			char **devs = NULL;
			int devs_size = cJSON_GetArraySize(pSDevs);
			if(devs_size > 0)
			{
				int j;
				pdevs_str = strings_alloc(devs_size);
				for(j=0; j<devs_size; j++)
				{
					strings_add(pdevs_str, 
						cJSON_GetArrayItem(pSDevs, j)->valuestring);
				}
				devs = pdevs_str->str;
			}

			cJSON *pSFuncs = cJSON_GetObjectItem(pScene, "func_ids");
			int *func_ids = NULL;
			int func_size = cJSON_GetArraySize(pSFuncs);
			if(func_size > 0)
			{
				int j;
				func_ids = (int *)calloc(func_size, sizeof(int));
				for(j=0; j<func_size; j++)
				{
					*(func_ids+j) = 
						atoi(cJSON_GetArrayItem(pSFuncs, j)->valuestring);
				}
			}

			cJSON *pSParams = cJSON_GetObjectItem(pScene, "params");
			strings_t *params_str = NULL;
			char **params = NULL;
			int params_size = cJSON_GetArraySize(pSParams);
			if(params_size > 0)
			{
				int j;
				params_str = strings_alloc(params_size);
				for(j=0; j<params_size; j++)
				{
					strings_add(params_str, 
						cJSON_GetArrayItem(pSParams, j)->valuestring);
				}
				params = params_str->str;
			}

			scene_t *scene = scene_create(name, devs, 
				devs_size, func_ids, func_size, params, params_size);

			scenes_put(scenes, scene);

			strings_free(pdevs_str);
			free(func_ids);
			strings_free(params_str);

			i++;
		}
		sync_scenes_with_user_sql(email, scenes);
		scenes_free(scenes);
	}

sync_end:
	cJSON_Delete(pRoot);
}

void sync_devices_with_user_sql(char *email, devices_t *devs)
{
	if(email == NULL || devs == NULL || devs->size == 0)
	{
		return;
	}

	SET_CMD_LINE("%s%s%s", 
		"SELECT devices FROM users WHERE email=\'", 
		email, 
		"\'");

	//DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
	if(sqlserver_excute_cmdline(GET_CMD_LINE()) < 0)
	{
		return;
	}

	if((mysql_res = mysql_store_result(&mysql_conn)) == NULL)
	{
		DE_PRINTF(1, "%s()%d : mysql store result failed\n\n", __FUNCTION__, __LINE__);
		return;
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

			int i;
			for(i=0; i<devs->size; i++)
			{
				int j = 0;
				int dev_size = cJSON_GetArraySize(pDeviceArray);
				while(j < dev_size)
				{
					cJSON *pDev = cJSON_GetArrayItem(pDeviceArray, j);
					char *sn = cJSON_GetObjectItem(pDev, "sn")->valuestring;

					if(!strcmp(*(devs->sn+i), sn))
					{
						cJSON_DeleteItemFromArray(pDeviceArray, j);
						break;
					}

					j++;
				}	
			}

			for(i=0; i<devs->size; i++)
			{	
				cJSON *t_dev = cJSON_CreateObject();
				cJSON_AddItemToArray(pDeviceArray, t_dev);
					
				cJSON_AddStringToObject(t_dev, "sn", *(devs->sn+i));
				if(*(devs->iscollects+i))
				{
					cJSON_AddStringToObject(t_dev, "iscollect", "0");
				}
				else
				{
					cJSON_AddStringToObject(t_dev, "iscollect", "1");
				}
				cJSON_AddStringToObject(t_dev, "locate", *(devs->locates+i));
			}

			char *devices = cJSON_Print(pRoot);

			SET_CMD_LINE("%s%s%s%s%s", 
						"UPDATE users SET devices=\'",
						devices,
						"\' WHERE email=\'",
						email,
						"\'");

			DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
			sqlserver_excute_cmdline(GET_CMD_LINE());

			cJSON_Delete(pRoot);
			free(devices);

			mysql_free_result(mysql_res);
			return;
		}
	}
}

void sync_areas_with_user_sql(char *email, areas_t *areas)
{
	if(email == NULL || areas == NULL || areas->size == 0)
	{
		return;
	}

	SET_CMD_LINE("%s%s%s", 
		"SELECT areas FROM users WHERE email=\'", 
		email, 
		"\'");

	//DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
	if(sqlserver_excute_cmdline(GET_CMD_LINE()) < 0)
	{
		return;
	}

	if((mysql_res = mysql_store_result(&mysql_conn)) == NULL)
	{
		DE_PRINTF(1, "%s()%d : mysql store result failed\n\n", __FUNCTION__, __LINE__);
		return;
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

			cJSON* pAreaArray = cJSON_GetObjectItem(pRoot, "areas");
			if (pAreaArray == NULL)
			{
				continue;
			}

			int i;
			for(i=0; i<areas->size; i++)
			{
				int j = 0;
				int area_size = cJSON_GetArraySize(pAreaArray);
				while(j < area_size)
				{
					char *area_str = cJSON_GetArrayItem(pAreaArray, j)->valuestring;

					if(!strcmp(*(areas->str+i), area_str))
					{
						cJSON_DeleteItemFromArray(pAreaArray, j);
						break;
					}

					j++;
				}	
			}

			for(i=0; i<areas->size; i++)
			{	
				cJSON *t_area = cJSON_CreateString(*(areas->str+i));
				cJSON_AddItemToArray(pAreaArray, t_area);
			}

			char *areas = cJSON_Print(pRoot);

			SET_CMD_LINE("%s%s%s%s%s", 
						"UPDATE users SET areas=\'",
						areas,
						"\' WHERE email=\'",
						email,
						"\'");

			DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
			sqlserver_excute_cmdline(GET_CMD_LINE());

			cJSON_Delete(pRoot);
			free(areas);

			mysql_free_result(mysql_res);
			return;
		}
	}
}

void sync_scenes_with_user_sql(char *email, scenes_t *scenes)
{
	if(email == NULL || scenes == NULL || scenes->size == 0)
	{
		return;
	}

	SET_CMD_LINE("%s%s%s", 
		"SELECT scenes FROM users WHERE email=\'", 
		email, 
		"\'");

	//DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
	if(sqlserver_excute_cmdline(GET_CMD_LINE()) < 0)
	{
		return;
	}

	if((mysql_res = mysql_store_result(&mysql_conn)) == NULL)
	{
		DE_PRINTF(1, "%s()%d : mysql store result failed\n\n", __FUNCTION__, __LINE__);
		return;
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

			cJSON* pSceneArray = cJSON_GetObjectItem(pRoot, "scenes");
			if (pSceneArray == NULL)
			{
				continue;
			}

			int i;
			for(i=0; i<scenes->size; i++)
			{
				int j = 0;
				int scene_size = cJSON_GetArraySize(pSceneArray);
				while(j < scene_size)
				{
					cJSON *pDev = cJSON_GetArrayItem(pSceneArray, j);
					char *name = cJSON_GetObjectItem(pDev, "name")->valuestring;

					if(!strcmp((*(scenes->scenes+i))->name, name))
					{
						cJSON_DeleteItemFromArray(pSceneArray, j);
						break;
					}

					j++;
				}	
			}

			for(i=0; i<scenes->size; i++)
			{	
				scene_t *scene = *(scenes->scenes+i);
				cJSON *t_scene = cJSON_CreateObject();
				cJSON_AddItemToArray(pSceneArray, t_scene);

				cJSON_AddStringToObject(t_scene, "name", scene->name);

				if(scene->dev_size > 0)
				{
					int j;
					cJSON *t_devices = cJSON_CreateArray();
					cJSON_AddItemToObject(t_scene, "devices", t_devices);

					for(j=0; j<scene->dev_size; j++)
					{
						cJSON *t_dev = cJSON_CreateString(*(scene->devices+j));
						cJSON_AddItemToArray(t_devices, t_dev);
					}
				}

				if(scene->func_size > 0)
				{
					int j;
					cJSON *t_funcs = cJSON_CreateArray();
					cJSON_AddItemToObject(t_scene, "func_ids", t_funcs);

					for(j=0; j<scene->func_size; j++)
					{
						char id_str[24] = {0};
						sprintf(id_str, "%d", *(scene->func_ids+j));
						cJSON *t_func_id = cJSON_CreateString(id_str);
						cJSON_AddItemToArray(t_funcs, t_func_id);
					}
				}

				if(scene->param_size > 0)
				{
					int j;
					cJSON *t_params = cJSON_CreateArray();
					cJSON_AddItemToObject(t_scene, "params", t_params);

					for(j=0; j<scene->param_size; j++)
					{
						cJSON *t_param = cJSON_CreateString(*(scene->params+j));
						cJSON_AddItemToArray(t_params, t_param);
					}
				}
			}

			char *devices = cJSON_Print(pRoot);

			SET_CMD_LINE("%s%s%s%s%s", 
						"UPDATE users SET scenes=\'",
						scenes,
						"\' WHERE email=\'",
						email,
						"\'");

			DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
			sqlserver_excute_cmdline(GET_CMD_LINE());

			cJSON_Delete(pRoot);
			free(devices);

			mysql_free_result(mysql_res);
			return;
		}
	}
}

void del_user_info_to_sql(char *data)
{
	char *email = NULL;
	cJSON* pDevs = NULL;
	cJSON* pAreas = NULL;
	cJSON* pScenes = NULL;

	if(data == NULL)
	{
		return;
	}

	cJSON *pRoot = cJSON_Parse(data);
	if(pRoot == NULL)
	{
		return;
	}

	cJSON *pEmail = cJSON_GetObjectItem(pRoot, "email");
	if(pEmail == NULL)
	{
		goto del_end;
	}

	email = pEmail->valuestring;

	pDevs = cJSON_GetObjectItem(pRoot, "devices");
	if (pDevs != NULL)
	{
		int dev_size = cJSON_GetArraySize(pDevs);
		devices_t *devs = devices_alloc(dev_size);
		int i = 0;
		while(i < dev_size)
		{
			cJSON *pDev = cJSON_GetArrayItem(pDevs, i);

			char *sn = cJSON_GetObjectItem(pDev, "sn")->valuestring;
			char *iscollect = cJSON_GetObjectItem(pDevs, "iscollect")->valuestring;
			char *locate = cJSON_GetObjectItem(pDevs, "locate")->valuestring;

			devices_add(devs, sn, atoi(iscollect), locate);

			i++;
		}
		del_devices_from_user_sql(email, devs);
		devices_free(devs);
	}

	pAreas = cJSON_GetObjectItem(pRoot, "areas");
	if (pAreas != NULL)
	{
		int area_size = cJSON_GetArraySize(pAreas);
		areas_t *areas = areas_alloc(area_size);
		int i = 0;
		while(i < area_size)
		{
			areas_add(areas, cJSON_GetArrayItem(pAreas, i)->valuestring);
			i++;
		}
		del_areas_from_user_sql(email, areas);
		strings_free(areas);
	}

	pScenes = cJSON_GetObjectItem(pRoot, "scenes");
	if (pScenes != NULL)
	{
		int scene_size = cJSON_GetArraySize(pScenes);
		scenes_t *scenes = scenes_alloc(scene_size);
		int i = 0;
		while(i < scene_size)
		{
			cJSON *pScene = cJSON_GetArrayItem(pScenes, i);

			char *name = cJSON_GetObjectItem(pScene, "name")->valuestring;

			cJSON *pSDevs = cJSON_GetObjectItem(pScene, "devices");
			strings_t *pdevs_str = NULL;
			char **devs = NULL;
			int devs_size = cJSON_GetArraySize(pSDevs);
			if(devs_size > 0)
			{
				int j;
				pdevs_str = strings_alloc(devs_size);
				for(j=0; j<devs_size; j++)
				{
					strings_add(pdevs_str, 
						cJSON_GetArrayItem(pSDevs, j)->valuestring);
				}
				devs = pdevs_str->str;
			}

			cJSON *pSFuncs = cJSON_GetObjectItem(pScene, "func_ids");
			int *func_ids = NULL;
			int func_size = cJSON_GetArraySize(pSFuncs);
			if(func_size > 0)
			{
				int j;
				func_ids = (int *)calloc(func_size, sizeof(int));
				for(j=0; j<func_size; j++)
				{
					*(func_ids+j) = 
						atoi(cJSON_GetArrayItem(pSFuncs, j)->valuestring);
				}
			}

			cJSON *pSParams = cJSON_GetObjectItem(pScene, "params");
			strings_t *params_str = NULL;
			char **params = NULL;
			int params_size = cJSON_GetArraySize(pSParams);
			if(params_size > 0)
			{
				int j;
				params_str = strings_alloc(params_size);
				for(j=0; j<params_size; j++)
				{
					strings_add(params_str, 
						cJSON_GetArrayItem(pSParams, j)->valuestring);
				}
				params = params_str->str;
			}

			scene_t *scene = scene_create(name, devs, 
				devs_size, func_ids, func_size, params, params_size);

			scenes_put(scenes, scene);

			strings_free(pdevs_str);
			free(func_ids);
			strings_free(params_str);

			i++;
		}
		del_scenes_from_user_sql(email, scenes);
		scenes_free(scenes);
	}

del_end:
	cJSON_Delete(pRoot);
}

void del_devices_from_user_sql(char *email, devices_t *devs)
{
	if(email == NULL || devs == NULL || devs->size == 0)
	{
		return;
	}

	SET_CMD_LINE("%s%s%s", 
		"SELECT devices FROM users WHERE email=\'", 
		email, 
		"\'");

	//DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
	if(sqlserver_excute_cmdline(GET_CMD_LINE()) < 0)
	{
		return;
	}

	if((mysql_res = mysql_store_result(&mysql_conn)) == NULL)
	{
		DE_PRINTF(1, "%s()%d : mysql store result failed\n\n", __FUNCTION__, __LINE__);
		return;
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

			int i;
			for(i=0; i<devs->size; i++)
			{
				int j = 0;
				int dev_size = cJSON_GetArraySize(pDeviceArray);
				while(j < dev_size)
				{
					cJSON *pDev = cJSON_GetArrayItem(pDeviceArray, j);
					char *sn = cJSON_GetObjectItem(pDev, "sn")->valuestring;

					if(!strcmp(*(devs->sn+i), sn))
					{
						cJSON_DeleteItemFromArray(pDeviceArray, j);
						break;
					}

					j++;
				}	
			}

			char *devices = cJSON_Print(pRoot);

			SET_CMD_LINE("%s%s%s%s%s", 
						"UPDATE users SET devices=\'",
						devices,
						"\' WHERE email=\'",
						email,
						"\'");

			DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
			sqlserver_excute_cmdline(GET_CMD_LINE());

			cJSON_Delete(pRoot);
			free(devices);

			mysql_free_result(mysql_res);
			return;
		}
	}
}

void del_areas_from_user_sql(char *email, areas_t *areas)
{
	if(email == NULL || areas == NULL || areas->size == 0)
	{
		return;
	}

	SET_CMD_LINE("%s%s%s", 
		"SELECT areas FROM users WHERE email=\'", 
		email, 
		"\'");

	//DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
	if(sqlserver_excute_cmdline(GET_CMD_LINE()) < 0)
	{
		return;
	}

	if((mysql_res = mysql_store_result(&mysql_conn)) == NULL)
	{
		DE_PRINTF(1, "%s()%d : mysql store result failed\n\n", __FUNCTION__, __LINE__);
		return;
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

			cJSON* pAreaArray = cJSON_GetObjectItem(pRoot, "areas");
			if (pAreaArray == NULL)
			{
				continue;
			}

			int i;
			for(i=0; i<areas->size; i++)
			{
				int j = 0;
				int area_size = cJSON_GetArraySize(pAreaArray);
				while(j < area_size)
				{
					char *area_str = cJSON_GetArrayItem(pAreaArray, j)->valuestring;

					if(!strcmp(*(areas->str+i), area_str))
					{
						cJSON_DeleteItemFromArray(pAreaArray, j);
						break;
					}

					j++;
				}	
			}

			char *areas = cJSON_Print(pRoot);

			SET_CMD_LINE("%s%s%s%s%s", 
						"UPDATE users SET areas=\'",
						areas,
						"\' WHERE email=\'",
						email,
						"\'");

			DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
			sqlserver_excute_cmdline(GET_CMD_LINE());

			cJSON_Delete(pRoot);
			free(areas);

			mysql_free_result(mysql_res);
			return;
		}
	}
}

void del_scenes_from_user_sql(char *email, scenes_t *scenes)
{
	if(email == NULL || scenes == NULL || scenes->size == 0)
	{
		return;
	}

	SET_CMD_LINE("%s%s%s", 
		"SELECT scenes FROM users WHERE email=\'", 
		email, 
		"\'");

	//DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
	if(sqlserver_excute_cmdline(GET_CMD_LINE()) < 0)
	{
		return;
	}

	if((mysql_res = mysql_store_result(&mysql_conn)) == NULL)
	{
		DE_PRINTF(1, "%s()%d : mysql store result failed\n\n", __FUNCTION__, __LINE__);
		return;
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

			cJSON* pSceneArray = cJSON_GetObjectItem(pRoot, "scenes");
			if (pSceneArray == NULL)
			{
				continue;
			}

			int i;
			for(i=0; i<scenes->size; i++)
			{
				int j = 0;
				int scene_size = cJSON_GetArraySize(pSceneArray);
				while(j < scene_size)
				{
					cJSON *pDev = cJSON_GetArrayItem(pSceneArray, j);
					char *name = cJSON_GetObjectItem(pDev, "name")->valuestring;

					if(!strcmp((*(scenes->scenes+i))->name, name))
					{
						cJSON_DeleteItemFromArray(pSceneArray, j);
						break;
					}

					j++;
				}	
			}

			char *devices = cJSON_Print(pRoot);

			SET_CMD_LINE("%s%s%s%s%s", 
						"UPDATE users SET scenes=\'",
						scenes,
						"\' WHERE email=\'",
						email,
						"\'");

			DE_PRINTF(1, "%s()%d : %s\n\n", __FUNCTION__, __LINE__ , GET_CMD_LINE());
			sqlserver_excute_cmdline(GET_CMD_LINE());

			cJSON_Delete(pRoot);
			free(devices);

			mysql_free_result(mysql_res);
			return;
		}
	}
}

int set_device_to_user_sql(char *email, char *dev_str)
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
	if(sqlserver_excute_cmdline(GET_CMD_LINE()) < 0)
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
			sqlserver_excute_cmdline(GET_CMD_LINE());

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
	sqlserver_excute_cmdline(GET_CMD_LINE());

	cJSON_Delete(pRoot);
	free(devices);

    mysql_free_result(mysql_res);
	return 1;
}
#endif

#ifdef __cplusplus
}
#endif

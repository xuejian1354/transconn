/*
 * request.c
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
#include "request.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sqlite3.h>
#include <cJSON.h>
#include <protocol/devices.h>
#include <services/balancer.h>
#include <module/dbopt.h>

extern char cmdline[CMDLINE_SIZE];

#ifdef COMM_CLIENT
void trans_send_tocolreq_request(frhandler_arg_t *arg, trfr_tocolreq_t *tocolreq)
{
	if(tocolreq == NULL)
	{
		return;
	}

	cJSON *pRoot = cJSON_CreateObject();
	cJSON_AddStringToObject(pRoot, JSON_FIELD_ACTION, get_action_to_str(tocolreq->action));

#ifdef TRANS_UDP_SERVICE	
	if(!strcmp(tocolreq->protocol, TRANSTOCOL_UDP))
	{
		cJSON_AddStringToObject(pRoot, JSON_FIELD_PROTOCOL, tocolreq->protocol);
		cJSON_AddStringToObject(pRoot, JSON_FIELD_RANDOM, tocolreq->random);
		char *frame = cJSON_Print(pRoot);
		socket_udp_sendto(&(arg->addr), frame, strlen(frame));
	}
#endif

#ifdef TRANS_TCP_CLIENT
	if(!strcmp(tocolreq->protocol, TRANSTOCOL_TCP))
	{
		cJSON_AddStringToObject(pRoot, JSON_FIELD_PROTOCOL, tocolreq->protocol);
		cJSON_AddStringToObject(pRoot, JSON_FIELD_RANDOM, tocolreq->random);
		char *frame = cJSON_Print(pRoot);
		socket_tcp_client_send(frame, strlen(frame));
	}
#endif

#ifdef TRANS_HTTP_REQUEST
	if(!strcmp(tocolreq->protocol, TRANSTOCOL_HTTP))
	{
		cJSON_AddStringToObject(pRoot, JSON_FIELD_PROTOCOL, tocolreq->protocol);
		cJSON_AddStringToObject(pRoot, JSON_FIELD_RANDOM, tocolreq->random);
		char *frame = cJSON_Print(pRoot);
		curl_http_request(CURL_POST, get_global_conf()->http_url, frame, curl_data);
	}
#endif

	cJSON_Delete(pRoot);
}

void trans_send_report_request(frhandler_arg_t *arg, trfr_report_t *report)
{
	if(report == NULL)
	{
		return;
	}

	char *frame;
	cJSON *pRoot = cJSON_CreateObject();

	cJSON_AddStringToObject(pRoot, JSON_FIELD_ACTION, get_action_to_str(report->action));
	cJSON_AddStringToObject(pRoot, JSON_FIELD_GWSN, report->gw_sn);

	int i = 0;
	cJSON *pDevs = NULL;
	if(report->dev_size > 0 && report->devices != NULL)
	{
		pDevs = cJSON_CreateArray();
		cJSON_AddItemToObject(pRoot, JSON_FIELD_DEVICES, pDevs);
	}

	while(i < report->dev_size)
	{
		
		cJSON *pDev = cJSON_CreateObject();
		cJSON_AddItemToArray(pDevs, pDev);

		cJSON_AddStringToObject(pDev, JSON_FIELD_NAME, (*(report->devices+i))->name);
		cJSON_AddStringToObject(pDev, JSON_FIELD_DEVSN, (*(report->devices+i))->dev_sn);
		char type[4] = {0};
		get_frapp_type_to_str(type, (*(report->devices+i))->dev_type);
		cJSON_AddStringToObject(pDev, JSON_FIELD_DEVTYPE, type);
		if((*(report->devices+i))->znet_status)
		{
			cJSON_AddStringToObject(pDev, JSON_FIELD_ZSTATUS, "1");
		}
		else
		{
			cJSON_AddStringToObject(pDev, JSON_FIELD_ZSTATUS, "0");
		}
		cJSON_AddStringToObject(pDev, JSON_FIELD_DEVDATA, (*(report->devices+i))->dev_data);
		
		i++;
	}

	cJSON_AddStringToObject(pRoot, JSON_FIELD_RANDOM, report->random);

	switch(get_trans_protocol())
	{
	case TOCOL_DISABLE:
		break;
	case TOCOL_ENABLE:
		break;
#ifdef TRANS_UDP_SERVICE
	case TOCOL_UDP:
		frame = cJSON_Print(pRoot);
		socket_udp_sendto(&(arg->addr), frame, strlen(frame));
		break;
#endif
#ifdef TRANS_TCP_CLIENT
	case TOCOL_TCP:
		frame = cJSON_Print(pRoot);
		socket_tcp_client_send(frame, strlen(frame));
		break;
#endif
#ifdef TRANS_HTTP_REQUEST
	case TOCOL_HTTP:
		frame = cJSON_Print(pRoot);
		curl_http_request(CURL_POST, get_global_conf()->http_url, frame, curl_data)
		break;
#endif
	}

	cJSON_Delete(pRoot);
}

void trans_send_check_request(frhandler_arg_t *arg, trfr_check_t *check)
{
	if(check == NULL)
	{
		return;
	}

	char *frame;
	cJSON *pRoot = cJSON_CreateObject();

	cJSON_AddStringToObject(pRoot, JSON_FIELD_ACTION, get_action_to_str(check->action));
	cJSON_AddStringToObject(pRoot, JSON_FIELD_GWSN, check->gw_sn);

	cJSON *pCode = cJSON_CreateObject();
	cJSON_AddItemToObject(pRoot, JSON_FIELD_CODE, pCode);

	cJSON_AddStringToObject(pCode, JSON_FIELD_CODECHECK, check->code.code_check);
	cJSON_AddStringToObject(pCode, JSON_FIELD_CODEDATA, check->code.code_data);
	cJSON_AddStringToObject(pRoot, JSON_FIELD_RANDOM, check->random);

	switch(get_trans_protocol())
	{
	case TOCOL_DISABLE:
		break;
	case TOCOL_ENABLE:
		break;
#ifdef TRANS_UDP_SERVICE
	case TOCOL_UDP:
		frame = cJSON_Print(pRoot);
		socket_udp_sendto(&(arg->addr), frame, strlen(frame));
		break;
#endif
#ifdef TRANS_TCP_CLIENT
	case TOCOL_TCP:
		frame = cJSON_Print(pRoot);
		socket_tcp_client_send(frame, strlen(frame));
		break;
#endif
#ifdef TRANS_HTTP_REQUEST
	case TOCOL_HTTP:
		frame = cJSON_Print(pRoot);
		curl_http_request(CURL_POST, get_global_conf()->http_url, frame, curl_data)
		break;
#endif
	}

	cJSON_Delete(pRoot);
}

void trans_send_respond_request(frhandler_arg_t *arg, trfr_respond_t *respond)
{}

void trans_refresh_handler(frhandler_arg_t *arg, trfr_refresh_t *refresh)
{}

void trans_control_handler(frhandler_arg_t *arg, trfr_control_t *control)
{}

void trans_tocolres_handler(frhandler_arg_t *arg, trfr_tocolres_t *tocolres)
{
	if(tocolres == NULL)
	{
		return;
	}

	switch(arg->transtocol)
	{
	case TOCOL_UDP:
		set_trans_protocol(TOCOL_UDP);
		break;

	case TOCOL_TCP:
		set_trans_protocol(TOCOL_TCP);
		break;
	case TOCOL_HTTP:
		set_trans_protocol(TOCOL_HTTP);
		break;
	}

	switch(tocolres->req_action)
	{
	case ACTION_REPORT:
	{
		dev_info_t *p_dev = get_gateway_info()->p_dev;
		while(p_dev != NULL)
		{
			p_dev->isdata_change = 0;
			p_dev = p_dev->next;
		}

		sql_set_datachange_zdev(get_gateway_info()->gw_no, 0);
	}
		break;
	}
}
#endif

#ifdef COMM_SERVER
void trans_tocolreq_handler(frhandler_arg_t *arg, trfr_tocolreq_t *tocolreq)
{
	if(tocolreq == NULL)
	{
		return;
	}

	if(tocolreq->action == ACTION_TOCOLREQ)
	{
		trfr_tocolres_t *tocolres = get_trfr_tocolres_alloc(tocolreq->action, tocolreq->random);
		trans_send_tocolres_request(arg, tocolres);
		get_trfr_tocolres_free(tocolres);
	}
}

void trans_report_handler(frhandler_arg_t *arg, trfr_report_t *report)
{
	if(report == NULL)
	{
		return;
	}

	if(!(get_trans_protocol() & arg->transtocol))
	{
		return;
	}

	if(report->action == ACTION_REPORT)
	{
		trfr_tocolres_t *tocolres = get_trfr_tocolres_alloc(report->action, report->random);
		trans_send_tocolres_request(arg, tocolres);
		get_trfr_tocolres_free(tocolres);
	}
}

void trans_check_handler(frhandler_arg_t *arg, trfr_check_t *check)
{
	if(check == NULL)
	{
		return;
	}

	if(!(get_trans_protocol() & arg->transtocol))
	{
		return;
	}

	if(check->action == ACTION_CHECK)
	{
		trfr_tocolres_t *tocolres = get_trfr_tocolres_alloc(check->action, check->random);
		trans_send_tocolres_request(arg, tocolres);
		get_trfr_tocolres_free(tocolres);
	}
}

void trans_respond_handler(frhandler_arg_t *arg, trfr_respond_t *respond)
{}

void trans_send_refresh_request(frhandler_arg_t *arg, trfr_refresh_t *refresh)
{}

void trans_send_control_request(frhandler_arg_t *arg, trfr_control_t *control)
{}

void trans_send_tocolres_request(frhandler_arg_t *arg, trfr_tocolres_t *tocolres)
{
	if(tocolres == NULL)
	{
		return;
	}

	char *frame = NULL;
	cJSON *pRoot = cJSON_CreateObject();
	cJSON_AddStringToObject(pRoot, JSON_FIELD_ACTION, get_action_to_str(tocolres->action));
	cJSON_AddStringToObject(pRoot, JSON_FIELD_REQACTION, get_action_to_str(tocolres->req_action));
	cJSON_AddStringToObject(pRoot, JSON_FIELD_RANDOM, tocolres->random);

	switch(arg->transtocol)
	{
	case TOCOL_UDP:
#ifdef TRANS_UDP_SERVICE
		frame = cJSON_Print(pRoot);
		socket_udp_sendto(&(arg->addr), frame, strlen(frame));
#endif
		break;

	case TOCOL_TCP:
#if defined(TRANS_TCP_SERVER)
		frame = cJSON_Print(pRoot);
		socket_tcp_server_send(arg, frame, strlen(frame));
#endif
		break;

	case TOCOL_HTTP:
		break;
	}
	
	cJSON_Delete(pRoot);
}
#endif

#ifdef COMM_CLIENT
void sync_gateway_info(gw_info_t *pgw_info)
{
	sql_add_gateway(pgw_info);
}

void sync_zdev_info(dev_info_t *pdev_info)
{
	if(pdev_info == NULL)
	{
		return;
	}

	sql_add_zdev(get_gateway_info(), pdev_info);

	if(pdev_info->isdata_change)
	{
		upload_data();
	}
}

void upload_data()
{
	transtocol_t transtocol = get_trans_protocol();
	if(transtocol == TOCOL_UDP
		|| transtocol == TOCOL_TCP
		|| transtocol == TOCOL_HTTP)
	{
		frhandler_arg_t arg;
		arg.transtocol = transtocol;
		arg.addr.sin_family = PF_INET;
		arg.addr.sin_port = htons(get_udp_port());
		arg.addr.sin_addr.s_addr = inet_addr(get_server_ip());

		char gwno_str[20] = {0};
		incode_xtocs(gwno_str,
						get_gateway_info()->gw_no,
						sizeof(zidentify_no_t));

		char *cur_code = gen_current_checkcode(NULL);
		if(strcmp(get_syncdata_checkcode(), cur_code))
		{
			set_syncdata_checkcode(cur_code);

			trfield_device_t **devices = NULL;
			int dev_size = 0;
#ifdef DB_API_WITH_SQLITE
			char gwsn[20] = {0};
			incode_xtocs(gwsn, get_gateway_info()->gw_no, sizeof(zidentify_no_t));
			SET_CMD_LINE("%s%s%s",
							"SELECT * FROM devices WHERE gwsn=\'",
							gwsn,
							"\' AND ischange=\'1\'");

		 	sqlite3_stmt *stmt;
		    if(sqlite3_prepare_v2(get_sqlite_db(), GET_CMD_LINE(), -1, &stmt, NULL) == SQLITE_OK)
		    {
		        while(sqlite3_step(stmt) == SQLITE_ROW)
				{
					if(sqlite3_column_count(stmt) > 17)
					{
						const char *name = sqlite3_column_text(stmt, 7);
						const char *dev_sn = sqlite3_column_text(stmt, 1);
						const char *dev_type = sqlite3_column_text(stmt, 2);
						int isonline = sqlite3_column_int(stmt, 10);
						const char *dev_data = sqlite3_column_text(stmt, 15);
						
						trfield_device_t *device = 
							get_trfield_device_alloc((char *)name,
														(char *)dev_sn,
														(char *)dev_type,
														isonline,
														(char *)dev_data);
						if(device != NULL)
						{
							if(devices == NULL)
							{
								devices = calloc(1, sizeof(trfield_device_t *));
								*devices = device;
							}
							else
							{
								devices = realloc(devices, (dev_size+1)*sizeof(trfield_device_t *));
								*(devices+dev_size) = device;
							}

							dev_size++;
						}
					}
				}
		    }
			sqlite3_finalize(stmt);
#else
			dev_info_t *p_dev = get_gateway_info()->p_dev;
			while(p_dev != NULL)
			{
				if(p_dev->isdata_change)
				{
					char *name = get_mix_name(p_dev->zapp_type,
												get_gateway_info()->gw_no[7],
												p_dev->zidentity_no[7]);

					sn_t dev_sn = {0};
					incode_xtocs(dev_sn, p_dev->zidentity_no, sizeof(zidentify_no_t));

					char dev_type[4] = {0};
					get_frapp_type_to_str(dev_type, p_dev->zapp_type);

					fr_buffer_t *data_buffer = get_devopt_data_tostr(p_dev->zdev_opt);
					char *dev_data = calloc(1, data_buffer->size+1);
					memcpy(dev_data, data_buffer->data, data_buffer->size);
					get_buffer_free(data_buffer);

					trfield_device_t *device = 
						get_trfield_device_alloc(name, dev_sn, dev_type, 1, dev_data);
					if(device != NULL)
					{
						if(devices == NULL)
						{
							devices = calloc(1, sizeof(trfield_device_t *));
							*devices = device;
						}
						else
						{
							devices = realloc(devices, (dev_size+1)*sizeof(trfield_device_t *));
							*(devices+dev_size) = device;
						}

						dev_size++;
					}
				}
				p_dev = p_dev->next;
			}
#endif
			trfr_report_t * report = get_trfr_report_alloc(gwno_str, devices, dev_size, NULL);

			trans_send_report_request(&arg, report);
			get_trfr_report_free(report);
		}
		else
		{
			trfr_check_t *t_check =
				get_trfr_check_alloc(gwno_str,
										NULL,
										0,
										"md5",
										cur_code,
										NULL);

			trans_send_check_request(&arg, t_check);
			get_trfr_check_free(t_check);
		}
		set_heartbeat_check(30);
	}
}
#endif


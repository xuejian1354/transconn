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
#include <cJSON.h>
#include <protocol/old/devices.h>
#include <protocol/common/mevent.h>
#include <services/balancer.h>
#include <module/serial.h>
#include <module/netlist.h>
#include <module/netapi.h>
#ifdef DB_API_WITH_SQLITE
#include <module/dbclient.h>
#endif
#ifdef DB_API_WITH_MYSQL
#include <module/dbserver.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TRANS_HTTP_REQUEST
char curl_buf[0x4000];
#endif

#ifdef COMM_CLIENT
respond_data_t *prespond_data = NULL;

void del_respond_data_with_sn(sn_t sn);
void device_ctrl_timer_callback(void *p);


void del_respond_data_with_sn(sn_t sn)
{
	respond_data_t *pre_data =  NULL;
	respond_data_t *t_data = prespond_data;


	while(t_data != NULL)
	{
		if(strcmp(t_data->sn, sn))
		{
			pre_data = t_data;
			t_data = t_data->next;
		}
		else
		{
			if(pre_data != NULL)
			{
				pre_data->next = t_data->next;
			}
			else
			{
				prespond_data = t_data->next;
			}

			free(t_data);
			return;
		}
	}
}

void trans_send_tocolreq_request(frhandler_arg_t *arg, trfr_tocolreq_t *tocolreq)
{
	if(tocolreq == NULL)
	{
		return;
	}

	cJSON *pRoot = cJSON_CreateObject();
	cJSON_AddStringToObject(pRoot, JSON_FIELD_ACTION, get_action_to_str(tocolreq->action));
	cJSON_AddStringToObject(pRoot, JSON_FIELD_GWSN, tocolreq->gw_sn);

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
		sprintf(curl_buf, "key=[%s]&datatype=%s\0", frame, get_action_to_str(ACTION_TOCOLREQ));
		curl_http_request(CURL_POST, get_global_conf()->http_url, curl_buf, curl_data);
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
		cJSON_AddStringToObject(pDev, JSON_FIELD_DEVTYPE, get_frapp_type_to_str((*(report->devices+i))->dev_type));
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

	trans_send_frame_request(arg, ACTION_REPORT, cJSON_Print(pRoot));

	cJSON_Delete(pRoot);
}

void trans_send_check_request(frhandler_arg_t *arg, trfr_check_t *check)
{
	if(check == NULL)
	{
		return;
	}

	cJSON *pRoot = cJSON_CreateObject();

	cJSON_AddStringToObject(pRoot, JSON_FIELD_ACTION, get_action_to_str(check->action));
	cJSON_AddStringToObject(pRoot, JSON_FIELD_GWSN, check->gw_sn);

	cJSON *pCodeArray = cJSON_CreateArray();
	cJSON *pCode = cJSON_CreateObject();
	cJSON_AddItemToArray(pCodeArray, pCode);
	cJSON_AddItemToObject(pRoot, JSON_FIELD_CODE, pCodeArray);

	cJSON_AddStringToObject(pCode, JSON_FIELD_CODECHECK, check->code.code_check);
	cJSON_AddStringToObject(pCode, JSON_FIELD_CODEDATA, check->code.code_data);
	cJSON_AddStringToObject(pRoot, JSON_FIELD_RANDOM, check->random);

	trans_send_frame_request(arg, ACTION_CHECK, cJSON_Print(pRoot));

	cJSON_Delete(pRoot);
}

void trans_send_respond_request(frhandler_arg_t *arg, trfr_respond_t *respond)
{
	if(respond == NULL)
	{
		return;
	}

	cJSON *pRoot = cJSON_CreateObject();

	cJSON_AddStringToObject(pRoot, JSON_FIELD_ACTION, get_action_to_str(respond->action));
	cJSON_AddStringToObject(pRoot, JSON_FIELD_GWSN, respond->gw_sn);
	cJSON_AddStringToObject(pRoot, JSON_FIELD_DEVSN, respond->dev_sn);
	cJSON_AddStringToObject(pRoot, JSON_FIELD_DEVDATA, respond->dev_data);
	cJSON_AddStringToObject(pRoot, JSON_FIELD_RANDOM, respond->random);

	trans_send_frame_request(arg, ACTION_RESPOND, cJSON_Print(pRoot));

	cJSON_Delete(pRoot);
}
#endif
#if defined(COMM_CLIENT) || defined(DE_TRANS_UDP_CONTROL)
void trans_refresh_handler(frhandler_arg_t *arg, trfr_refresh_t *refresh)
{
	if(refresh == NULL)
	{
		return;
	}

#ifdef COMM_CLIENT
	zidentify_no_t gw_sn;
	incode_ctoxs(gw_sn, refresh->gw_sn, 16);
	if(memcmp(gw_sn, get_gateway_info()->gw_no, sizeof(zidentify_no_t)))
	{
		return;
	}
	
	if(refresh->dev_sns != NULL && refresh->sn_size > 0)
	{
		int i;
		for(i=0; i<refresh->sn_size; i++)
		{
			zidentify_no_t dev_no;
			incode_ctoxs(dev_no, (char *)(refresh->dev_sns+i), 16);
			sqlclient_set_datachange_zdev(dev_no, 1);
		}
	}
	else
	{
		sqlclient_set_datachange_zdev(get_gateway_info()->gw_no, 1);
	}

	upload_data(1, refresh->random);

#elif defined(DE_TRANS_UDP_CONTROL)
	if(arg == NULL || refresh->obj == NULL)
	{
		return;
	}
	if(ntohs(arg->addr.sin_port) == DE_UDP_CTRL_PORT
		&& !strcmp(refresh->obj->owner, INFO_OBJECT_DEBUG))
	{
		frhandler_arg_t *t_arg = NULL;
		char *protocol = sqlserver_get_column_from_gwsn("transtocol", refresh->gw_sn);
#ifdef TRANS_UDP_SERVICE
		if(!strcmp(protocol, "udp"))
		{
			t_arg = (frhandler_arg_t *)calloc(1, sizeof(frhandler_arg_t));

			t_arg->transtocol = TOCOL_UDP;
			t_arg->addr.sin_family = PF_INET;
			t_arg->addr.sin_port =
				htons(atoi(sqlserver_get_column_from_gwsn("udp_port", refresh->gw_sn)));
			t_arg->addr.sin_addr.s_addr =
				inet_addr(sqlserver_get_column_from_gwsn("ip", refresh->gw_sn));
		}
#endif
#ifdef TRANS_TCP_SERVER
		if(!strcmp(protocol, "tcp"))
		{
			char ipaddr[24] = {0};
			int port = atoi(sqlserver_get_column_from_gwsn("tcp_port", refresh->gw_sn));
			sprintf(ipaddr, "%s:%u",
				sqlserver_get_column_from_gwsn("ip", refresh->gw_sn), 
				port);

			tcp_conn_t *tconn = queryfrom_tcpconn_list_with_ipaddr(ipaddr);

			if(tconn != NULL)
			{
				t_arg = (frhandler_arg_t *)calloc(1, sizeof(frhandler_arg_t));
				t_arg->transtocol = TOCOL_TCP;
				t_arg->fd = tconn->fd;
			}
		}
#endif
		if(!strcmp(refresh->obj->owner, INFO_OBJECT_DEBUG))
		{
			trans_send_frame_request(t_arg, ACTION_REFRESH, arg->buf);
		}
		free(t_arg);
	}
#endif
}

void trans_control_handler(frhandler_arg_t *arg, trfr_control_t *control)
{
	if(control == NULL)
	{
		return;
	}

#ifdef COMM_CLIENT
	zidentify_no_t gw_sn;
	incode_ctoxs(gw_sn, control->gw_sn, 16);
	if(memcmp(gw_sn, get_gateway_info()->gw_no, sizeof(zidentify_no_t)))
	{
		return;
	}

	trfield_ctrl_t **ctrls = control->ctrls;
	if(ctrls == NULL || control->ctrl_size <= 0)
	{
		return;
	}

	int i;
	for(i=0; i<control->ctrl_size; i++)
	{
		trfield_ctrl_t *ctrl = *(ctrls + i);
		if(ctrl != NULL)
		{
			device_ctrl(ctrl->dev_sn,
							ctrl->cmd,
							control->random,
							trans_send_respond_request);
			usleep(100);
		}
	}

#elif defined(DE_TRANS_UDP_CONTROL)
	if(arg == NULL || control->obj == NULL)
	{
		return;
	}
	if(ntohs(arg->addr.sin_port) == DE_UDP_CTRL_PORT
		&& !strcmp(control->obj->owner, INFO_OBJECT_DEBUG))
	{
		frhandler_arg_t *t_arg = NULL;
		char *protocol = sqlserver_get_column_from_gwsn("transtocol", control->gw_sn);
#ifdef TRANS_UDP_SERVICE
		if(!strcmp(protocol, "udp"))
		{
			t_arg = (frhandler_arg_t *)calloc(1, sizeof(frhandler_arg_t));

			t_arg->transtocol = TOCOL_UDP;
			t_arg->addr.sin_family = PF_INET;
			t_arg->addr.sin_port =
				htons(atoi(sqlserver_get_column_from_gwsn("udp_port", control->gw_sn)));
			t_arg->addr.sin_addr.s_addr =
				inet_addr(sqlserver_get_column_from_gwsn("ip", control->gw_sn));
		}
#endif
#ifdef TRANS_TCP_SERVER
		else if(!strcmp(protocol, "tcp"))
		{
			char ipaddr[24] = {0};
			int port = atoi(sqlserver_get_column_from_gwsn("tcp_port", control->gw_sn));
			sprintf(ipaddr, "%s:%u",
				sqlserver_get_column_from_gwsn("ip", control->gw_sn), 
				port);

			tcp_conn_t *tconn = queryfrom_tcpconn_list_with_ipaddr(ipaddr);
			if(tconn != NULL)
			{
				t_arg = (frhandler_arg_t *)calloc(1, sizeof(frhandler_arg_t));
				t_arg->transtocol = TOCOL_TCP;
				t_arg->fd = tconn->fd;
			}
		}
#endif
		if(!strcmp(control->obj->owner, INFO_OBJECT_DEBUG))
		{
			trans_send_frame_request(t_arg, ACTION_CONTROL, arg->buf);
		}
		free(t_arg);
	}
#endif
}
#endif

void trans_tocolres_handler(frhandler_arg_t *arg, trfr_tocolres_t *tocolres)
{
	if(tocolres == NULL)
	{
		return;
	}

#ifdef COMM_CLIENT
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

		sqlclient_set_datachange_zdev(get_gateway_info()->gw_no, 0);
	}
		break;
	}
#endif
}

#ifdef COMM_SERVER
void trans_tocolreq_handler(frhandler_arg_t *arg, trfr_tocolreq_t *tocolreq)
{
	if(tocolreq == NULL)
	{
		return;
	}

#ifdef DB_API_WITH_MYSQL
	sqlserver_add_gateway(arg, tocolreq->gw_sn);
#endif

	if(tocolreq->action == ACTION_TOCOLREQ)
	{
		trfr_tocolres_t *tocolres = get_trfr_tocolres_alloc(NULL,
															tocolreq->action,
															NULL,
															tocolreq->random);
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

#ifdef DB_API_WITH_MYSQL
	sqlserver_add_gateway(arg, report->gw_sn);
	sqlserver_add_zdevices(arg, report);
#endif

	if(!(get_trans_protocol() & arg->transtocol))
	{
		return;
	}

	if(report->action == ACTION_REPORT)
	{
		trfr_tocolres_t *tocolres = get_trfr_tocolres_alloc(NULL,
																report->action,
																NULL,
																report->random);
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

#ifdef DB_API_WITH_MYSQL
	sqlserver_add_gateway(arg, check->gw_sn);
#endif

	if(!(get_trans_protocol() & arg->transtocol))
	{
		return;
	}

	if(check->action == ACTION_CHECK)
	{
		trfr_tocolres_t *tocolres = get_trfr_tocolres_alloc(NULL,
																check->action,
																NULL,
																check->random);
		trans_send_tocolres_request(arg, tocolres);
		get_trfr_tocolres_free(tocolres);
	}
}

void trans_respond_handler(frhandler_arg_t *arg, trfr_respond_t *respond)
{
	if(respond == NULL)
	{
		return;
	}

#ifdef DB_API_WITH_MYSQL
	sqlserver_add_gateway(arg, respond->gw_sn);
	sqlserver_update_zdevice(arg, respond);
#endif

	if(!(get_trans_protocol() & arg->transtocol))
	{
		return;
	}

	if(respond->action == ACTION_RESPOND)
	{
		trfr_tocolres_t *tocolres = get_trfr_tocolres_alloc(NULL,
																respond->action,
																NULL,
																respond->random);
		trans_send_tocolres_request(arg, tocolres);
		get_trfr_tocolres_free(tocolres);
	}
}

void trans_send_refresh_request(frhandler_arg_t *arg, trfr_refresh_t *refresh)
{
	if(refresh == NULL)
	{
		return;
	}

	cJSON *pRoot = cJSON_CreateObject();
	cJSON_AddStringToObject(pRoot, JSON_FIELD_ACTION, get_action_to_str(refresh->action));

	if(refresh->obj != NULL)
	{
		cJSON *pObj = cJSON_CreateObject();
		cJSON_AddItemToObject(pRoot, JSON_FIELD_OBJECT, pObj);

		cJSON_AddStringToObject(pObj, JSON_FIELD_OWNER, refresh->obj->owner);
		cJSON_AddStringToObject(pObj, JSON_FIELD_CUSTOM, refresh->obj->custom);
	}

	cJSON_AddStringToObject(pRoot, JSON_FIELD_GWSN, refresh->gw_sn);

	cJSON *pDevSNs = NULL;
	if(refresh->sn_size > 0 && refresh->dev_sns != NULL)
	{
		pDevSNs = cJSON_CreateArray();
		cJSON_AddItemToObject(pRoot, JSON_FIELD_DEVSNS, pDevSNs);
	}

	int i = 0;
	while(i < refresh->sn_size)
	{
		cJSON *pDevSN = cJSON_CreateString(*(refresh->dev_sns+i));
		cJSON_AddItemToArray(pDevSNs, pDevSN);

		i++;
	}

	cJSON_AddStringToObject(pRoot, JSON_FIELD_RANDOM, refresh->random);

	trans_send_frame_request(arg, ACTION_REFRESH, cJSON_Print(pRoot));

	cJSON_Delete(pRoot);
}

void trans_send_control_request(frhandler_arg_t *arg, trfr_control_t *control)
{
	if(control == NULL)
	{
		return;
	}

	cJSON *pRoot = cJSON_CreateObject();
	cJSON_AddStringToObject(pRoot, JSON_FIELD_ACTION, get_action_to_str(control->action));

	if(control->obj != NULL)
	{
		cJSON *pObj = cJSON_CreateObject();
		cJSON_AddItemToObject(pRoot, JSON_FIELD_OBJECT, pObj);

		cJSON_AddStringToObject(pObj, JSON_FIELD_OWNER, control->obj->owner);
		cJSON_AddStringToObject(pObj, JSON_FIELD_CUSTOM, control->obj->custom);
	}

	cJSON_AddStringToObject(pRoot, JSON_FIELD_GWSN, control->gw_sn);

	int i = 0;
	cJSON *pCtrls = NULL;
	if(control->ctrl_size > 0 && control->ctrls != NULL)
	{
		pCtrls = cJSON_CreateArray();
		cJSON_AddItemToObject(pRoot, JSON_FIELD_CTRLS, pCtrls);
	}

	while(i < control->ctrl_size)
	{
		trfield_ctrl_t *ctrl = *(control->ctrls+i);
		if(ctrl != NULL)
		{
			cJSON *pCtrl = cJSON_CreateObject();
			cJSON_AddItemToArray(pCtrls, pCtrl);

			cJSON_AddStringToObject(pCtrl, JSON_FIELD_DEVSN, ctrl->dev_sn);
			cJSON_AddStringToObject(pCtrl, JSON_FIELD_CMD, ctrl->cmd);
		}

		i++;
	}

	cJSON_AddStringToObject(pRoot, JSON_FIELD_RANDOM, control->random);

	trans_send_frame_request(arg, ACTION_CONTROL, cJSON_Print(pRoot));

	cJSON_Delete(pRoot);
}
#endif

void trans_send_tocolres_request(frhandler_arg_t *arg, trfr_tocolres_t *tocolres)
{
	if(tocolres == NULL)
	{
		return;
	}

	char *frame = NULL;
	cJSON *pRoot = cJSON_CreateObject();
	cJSON_AddStringToObject(pRoot, JSON_FIELD_ACTION, get_action_to_str(tocolres->action));

	if(tocolres->obj)
	{
		cJSON *pObj = cJSON_CreateObject();
		cJSON_AddItemToObject(pRoot, JSON_FIELD_OBJECT, pObj);

		cJSON_AddStringToObject(pObj, JSON_FIELD_OWNER, tocolres->obj->owner);
		cJSON_AddStringToObject(pObj, JSON_FIELD_CUSTOM, tocolres->obj->custom);
	}

	cJSON_AddStringToObject(pRoot, JSON_FIELD_REQACTION, get_action_to_str(tocolres->req_action));
	if(strlen(tocolres->info))
	{
		cJSON_AddStringToObject(pRoot, JSON_FIELD_INFO, tocolres->info);
	}
	cJSON_AddStringToObject(pRoot, JSON_FIELD_RANDOM, tocolres->random);

	trans_send_frame_request(arg, ACTION_TOCOLRES, cJSON_Print(pRoot));
	
	cJSON_Delete(pRoot);
}

#ifdef COMM_CLIENT
void sync_gateway_info(gw_info_t *pgw_info)
{
	sqlclient_add_gateway(pgw_info);
}

void sync_zdev_info(uint8 isrefresh, dev_info_t *pdev_info)
{
	if(pdev_info == NULL)
	{
		return;
	}

	sqlclient_add_zdev(get_gateway_info(), pdev_info);

	sn_t dev_sn = {0};
	incode_xtocs(dev_sn, pdev_info->zidentity_no, sizeof(zidentify_no_t));

	respond_data_t *trespond_data = prespond_data;
	while(trespond_data != NULL)
	{
		if(!strcmp(trespond_data->sn, dev_sn))
		{
			sn_t gw_sn = {0};
			incode_xtocs(gw_sn, get_gateway_info()->gw_no, sizeof(zidentify_no_t));

			bzero(trespond_data->dev_data, JSON_FIELD_DATA_MAXSIZE);
			fr_buffer_t *buffer = get_devopt_data_tostr(pdev_info->zdev_opt);
			if(buffer != NULL)
			{
				STRS_MEMCPY(trespond_data->dev_data,
								buffer->data,
								sizeof(trespond_data->dev_data),
								buffer->size);
			}
			get_buffer_free(buffer);

			trfr_respond_t *respond =
				get_trfr_respond_alloc(NULL,
										gw_sn,
										dev_sn,
										trespond_data->dev_data,
										trespond_data->random);

			trespond_data->respond_callback(get_transtocol_frhandler_arg(), respond);
			get_trfr_respond_free(respond);

			del_timer_event(trespond_data->timer_id);
			del_respond_data_with_sn(dev_sn);
			return;
		}

		trespond_data = trespond_data->next;
	}

	if(pdev_info->isdata_change)
	{
		upload_data(isrefresh, NULL);
	}
}

void upload_data(uint8 isrefresh, char *random)
{
	uint16 transtocol = get_trans_protocol();
	if(transtocol == TOCOL_UDP
		|| transtocol == TOCOL_TCP
		|| transtocol == TOCOL_HTTP
		|| transtocol == TOCOL_WS)
	{
		sn_t gwno_str = {0};
		incode_xtocs(gwno_str,
						get_gateway_info()->gw_no,
						sizeof(zidentify_no_t));

		char *cur_code = gen_current_checkcode(NULL);
		if(isrefresh || strcmp(get_syncdata_checkcode(), cur_code))
		{
			set_syncdata_checkcode(cur_code);

			trfield_device_t **devices = NULL;
			int dev_size = 0;
#ifdef DB_API_WITH_SQLITE
			sqlclient_get_zdevices(isrefresh, &devices, &dev_size);
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

					fr_buffer_t *data_buffer = get_devopt_data_tostr(p_dev->zdev_opt);
					char *dev_data = NULL;
					if(data_buffer != NULL)
					{
						dev_data = (char *)calloc(1, data_buffer->size+1);
						memcpy(dev_data, data_buffer->data, data_buffer->size);
					}
					get_buffer_free(data_buffer);

					trfield_device_t *device = 
						get_trfield_device_alloc(name,
													dev_sn,
													get_frapp_type_to_str(p_dev->zapp_type),
													1,
													dev_data);
					if(device != NULL)
					{
						if(devices == NULL)
						{
							devices = (trfield_device_t **)calloc(1, sizeof(trfield_device_t *));
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
			trfr_report_t * report = get_trfr_report_alloc(NULL,
															gwno_str,
															devices,
															dev_size,
															random);

			trans_send_report_request(get_transtocol_frhandler_arg(), report);
			get_trfr_report_free(report);
		}
		else
		{
			trfr_check_t *t_check =
				get_trfr_check_alloc(NULL,
										gwno_str,
										NULL,
										0,
										(char *)"md5",
										cur_code,
										NULL);

			trans_send_check_request(get_transtocol_frhandler_arg(), t_check);
			get_trfr_check_free(t_check);
		}
	}
}

void device_ctrl(sn_t sn, char *cmd, char *random, respond_request_t callback)
{
	if(sn == NULL || cmd == NULL)
	{
		return;
	}

	int cmd_len = strlen(cmd);
	if(cmd_len > FRAME_DATA_SIZE)
	{
		return;
	}

	uint16 znet_addr = get_znet_addr_with_sn(sn);
	int timer_id = (ZDEVICE_RESPOND_EVENT<<16)+atoi(get_md5(get_system_time(), 2));

	respond_data_t *mrespond_data = (respond_data_t *)calloc(1, sizeof(respond_data_t));

	strcpy(mrespond_data->sn, sn);
	strcpy(mrespond_data->random, random);
	mrespond_data->respond_callback = callback;
	mrespond_data->timer_id = timer_id;
	mrespond_data->next = NULL;

	if(prespond_data == NULL)
	{
		prespond_data = mrespond_data;
	}
	else
	{
		respond_data_t *trespond_data = prespond_data;
		while(trespond_data->next != NULL)
		{
			trespond_data = trespond_data->next;
		}
		trespond_data->next = mrespond_data;
	}

	timer_event_param_t param;
	param.resident = 0;
	param.interval = 3000;
	param.count = 1;
	param.immediate = 0;
	param.arg = mrespond_data;
	set_mevent(timer_id, device_ctrl_timer_callback, &param);

	if(znet_addr)
	{
		int size = FR_DE_DATA_FIX_LEN + cmd_len;
		uint8 *buffer = (uint8 *)calloc(size, sizeof(uint8));

		memcpy(buffer, FR_HEAD_DE, 2);
		memcpy(buffer+2, FR_CMD_SINGLE_EXCUTE, 4);
		sprintf((char *)(buffer+6), "%04X", znet_addr);
		memcpy(buffer+10, cmd, cmd_len);
		memcpy(buffer+10+cmd_len, FR_TAIL, 4);

		serial_write((char *)buffer, size);
		free(buffer);
	}
}

void device_ctrl_timer_callback(void *p)
{
	respond_data_t *mrespond_data = (respond_data_t *)p;
	if(mrespond_data != NULL)
	{
		trfr_tocolres_t *tocolres = 
			get_trfr_tocolres_alloc(NULL,
										ACTION_CONTROL,
										(char *)INFO_TIMEOUT,
										mrespond_data->random);
		trans_send_tocolres_request(get_transtocol_frhandler_arg(), tocolres);
		get_trfr_tocolres_free(tocolres);

		del_respond_data_with_sn(mrespond_data->sn);
	}
}
#endif

#ifdef DE_TRANS_UDP_CONTROL
void detrans_send_refresh(sn_t devsn)
{
	frhandler_arg_t *arg = NULL;

	sn_t gwsn = {0};
	char *pgwsn = sqlserver_get_column_from_zdevice("gw_sn", devsn);
	STRS_MEMCPY(gwsn, pgwsn, sizeof(gwsn), strlen(pgwsn));

	char *protocol = sqlserver_get_column_from_gwsn("transtocol", gwsn);
	if(protocol == NULL)
	{
		return;
	}

#ifdef TRANS_UDP_SERVICE
	if(!strcmp(protocol, TRANSTOCOL_UDP))
	{
		arg = (frhandler_arg_t *)calloc(1, sizeof(frhandler_arg_t));

		arg->transtocol = TOCOL_UDP;
		arg->addr.sin_family = PF_INET;
		arg->addr.sin_port =
			htons(atoi(sqlserver_get_column_from_gwsn("udp_port", gwsn)));
		arg->addr.sin_addr.s_addr =
			inet_addr(sqlserver_get_column_from_gwsn("ip", gwsn));
	}
#endif
#ifdef TRANS_TCP_SERVER
	if(!strcmp(protocol, TRANSTOCOL_TCP))
	{
		char ipaddr[24] = {0};
		int port = atoi(sqlserver_get_column_from_gwsn("tcp_port", gwsn));
		sprintf(ipaddr, "%s:%u",
			sqlserver_get_column_from_gwsn("ip", gwsn), 
			port);

		tcp_conn_t *tconn = queryfrom_tcpconn_list_with_ipaddr(ipaddr);

		if(tconn != NULL)
		{
			arg = (frhandler_arg_t *)calloc(1, sizeof(frhandler_arg_t));
			arg->transtocol = TOCOL_TCP;
			arg->fd = tconn->fd;
		}
	}
#endif

	trfield_obj_t obj = {
		INFO_OBJECT_DEBUG,
		INFO_OBJECT_DEVICE,
	};
	
	trfr_refresh_t refresh;
	refresh.action = ACTION_REFRESH;
	refresh.obj = &obj;
	STRS_MEMCPY(refresh.gw_sn, gwsn, sizeof(sn_t), strlen(gwsn));
	refresh.dev_sns = (char (*)[JSON_FIELD_SN_MAXSIZE])devsn;
	refresh.sn_size = 1;
	STRS_MEMCPY(refresh.random,
					get_md5(get_system_time(), 2),
					sizeof(refresh.random),
					4);

	trans_send_refresh_request(arg, &refresh);

	free(arg);
}

void detrans_send_control(sn_t devsn, char *cmd)
{
	frhandler_arg_t *arg = NULL;

	sn_t gwsn = {0};
	char *pgwsn = sqlserver_get_column_from_zdevice("gw_sn", devsn);
	STRS_MEMCPY(gwsn, pgwsn, sizeof(gwsn), strlen(pgwsn));

	char *protocol = sqlserver_get_column_from_gwsn("transtocol", gwsn);
	if(protocol == NULL)
	{
		return;
	}
#ifdef TRANS_UDP_SERVICE
	if(!strcmp(protocol, TRANSTOCOL_UDP))
	{
		arg = (frhandler_arg_t *)calloc(1, sizeof(frhandler_arg_t));

		arg->transtocol = TOCOL_UDP;
		arg->addr.sin_family = PF_INET;
		arg->addr.sin_port =
			htons(atoi(sqlserver_get_column_from_gwsn("udp_port", gwsn)));
		arg->addr.sin_addr.s_addr =
			inet_addr(sqlserver_get_column_from_gwsn("ip", gwsn));
	}
#endif
#ifdef TRANS_TCP_SERVER
	if(!strcmp(protocol, TRANSTOCOL_TCP))
	{
		char ipaddr[24] = {0};
		int port = atoi(sqlserver_get_column_from_gwsn("tcp_port", gwsn));
		sprintf(ipaddr, "%s:%u",
			sqlserver_get_column_from_gwsn("ip", gwsn), 
			port);

		tcp_conn_t *tconn = queryfrom_tcpconn_list_with_ipaddr(ipaddr);

		if(tconn != NULL)
		{
			arg = (frhandler_arg_t *)calloc(1, sizeof(frhandler_arg_t));
			arg->transtocol = TOCOL_TCP;
			arg->fd = tconn->fd;
		}
	}
#endif

	trfield_obj_t obj = {
		INFO_OBJECT_DEBUG,
		INFO_OBJECT_DEVICE,
	};

	trfield_ctrl_t ctrl;
	STRS_MEMCPY(ctrl.dev_sn, devsn, sizeof(ctrl.dev_sn), strlen(devsn));
	STRS_MEMCPY(ctrl.cmd, cmd, sizeof(ctrl.cmd), strlen(cmd));

	trfield_ctrl_t *pctrl = &ctrl;
	
	trfr_control_t control;
	control.action = ACTION_CONTROL;
	control.obj = &obj;
	STRS_MEMCPY(control.gw_sn, gwsn, sizeof(control.gw_sn), strlen(gwsn));
	control.ctrls = &pctrl;
	control.ctrl_size = 1;
	STRS_MEMCPY(control.random,
					get_md5(get_system_time(), 2),
					sizeof(control.random),
					4);

	trans_send_control_request(arg, &control);

	free(arg);
}
#endif

void trans_send_frame_request(frhandler_arg_t *arg, trans_action_t action, char *frame)
{
#ifdef COMM_SERVER
	if(arg == NULL)
	{
		return;
	}

	switch(arg->transtocol)
#elif defined(COMM_CLIENT)
	switch(get_trans_protocol())
#endif
	{
	case TOCOL_DISABLE:
		break;

	case TOCOL_ENABLE:
		break;

	case TOCOL_UDP:
#ifdef TRANS_UDP_SERVICE
		socket_udp_sendto(&(arg->addr), frame, strlen(frame));
#ifdef COMM_CLIENT
		set_heartbeat_check(0, get_global_conf()->udp_timeout);
#endif
#endif
		break;

	case TOCOL_TCP:
#ifdef TRANS_TCP_SERVER
		socket_tcp_server_send(arg, frame, strlen(frame));
#elif defined(TRANS_TCP_CLIENT)
		socket_tcp_client_send(frame, strlen(frame));
		set_heartbeat_check(0, get_global_conf()->tcp_timeout);
#endif
		break;

	case TOCOL_HTTP:
#ifdef TRANS_HTTP_REQUEST
		sprintf(curl_buf, "key=[%s]&datatype=%s\0", frame, get_action_to_str(action));
		curl_http_request(CURL_POST, get_global_conf()->http_url, curl_buf, curl_data);
		set_heartbeat_check(0, get_global_conf()->http_timeout);
#endif
		break;

	case TOCOL_WS:
#ifdef TRANS_WS_CONNECT
		ws_send(frame, strlen(frame));
		set_heartbeat_check(0, get_global_conf()->ws_timeout);
#endif
		break;
	}
}

#ifdef __cplusplus
}
#endif


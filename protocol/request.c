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
#include <protocol/common/mevent.h>
#include <module/serial.h>
#include <module/netlist.h>
#include <module/netapi.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TRANS_HTTP_REQUEST
char curl_buf[0x4000];
#endif

#ifdef COMM_TARGET
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
	if(!strcmp(tocolreq->protocol, "udp"))
	{
		cJSON_AddStringToObject(pRoot, JSON_FIELD_PROTOCOL, tocolreq->protocol);
		cJSON_AddStringToObject(pRoot, JSON_FIELD_RANDOM, tocolreq->random);
		char *frame = cJSON_Print(pRoot);
		socket_udp_sendto(&(arg->addr), frame, strlen(frame));
	}
#endif

#ifdef TRANS_TCP_CLIENT
	if(!strcmp(tocolreq->protocol, "tcp"))
	{
		cJSON_AddStringToObject(pRoot, JSON_FIELD_PROTOCOL, tocolreq->protocol);
		cJSON_AddStringToObject(pRoot, JSON_FIELD_RANDOM, tocolreq->random);
		char *frame = cJSON_Print(pRoot);
		socket_tcp_client_send(frame, strlen(frame));
	}
#endif

#ifdef TRANS_HTTP_REQUEST
	if(!strcmp(tocolreq->protocol, "http"))
	{
		cJSON_AddStringToObject(pRoot, JSON_FIELD_PROTOCOL, tocolreq->protocol);
		cJSON_AddStringToObject(pRoot, JSON_FIELD_RANDOM, tocolreq->random);
		char *frame = cJSON_Print(pRoot);
		sprintf(curl_buf, "key=[%s]&datatype=%s\0", frame, get_action_to_str(ACTION_TOCOLREQ));
		curl_http_request(CURL_POST, curl_buf, curl_data);
	}
#endif

#ifdef TRANS_WS_CONNECT
	if(!strcmp(tocolreq->protocol, "websocket"))
	{
		cJSON_AddStringToObject(pRoot, JSON_FIELD_PROTOCOL, tocolreq->protocol);
		cJSON_AddStringToObject(pRoot, JSON_FIELD_RANDOM, tocolreq->random);
		char *frame = cJSON_Print(pRoot);
		//sprintf(curl_buf, "key=[%s]&datatype=%s\0", frame, get_action_to_str(ACTION_TOCOLREQ));
		ws_send(frame, strlen(frame));
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
		//cJSON_AddStringToObject(pDev, JSON_FIELD_DEVTYPE, get_frapp_type_to_str((*(report->devices+i))->dev_type));
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
#if defined(COMM_TARGET)
void trans_refresh_handler(frhandler_arg_t *arg, trfr_refresh_t *refresh)
{
	if(refresh == NULL)
	{
		return;
	}

#ifdef COMM_TARGET
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
		}
	}
	else
	{
	}

	upload_data(1, refresh->random);
#endif
}

void trans_control_handler(frhandler_arg_t *arg, trfr_control_t *control)
{
	if(control == NULL)
	{
		return;
	}

#ifdef COMM_TARGET
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
#endif
}
#endif

void trans_tocolres_handler(frhandler_arg_t *arg, trfr_tocolres_t *tocolres)
{
	if(tocolres == NULL)
	{
		return;
	}

#ifdef COMM_TARGET
	switch(tocolres->req_action)
	{
	case ACTION_REPORT:
	{
		dev_info_t *p_dev = get_gateway_info()->p_dev;
		while(p_dev != NULL)
		{
			p_dev->ischange = 0;
			p_dev = p_dev->next;
		}
	}
		break;
	}
#endif
}

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

#ifdef COMM_TARGET
void sync_gateway_info(gw_info_t *pgw_info)
{
}

void sync_zdev_info(uint8 isrefresh, dev_info_t *pdev_info)
{
	if(pdev_info == NULL)
	{
		return;
	}

	sn_t dev_sn = {0};
	incode_xtocs(dev_sn, pdev_info->dev_no, sizeof(zidentify_no_t));

	respond_data_t *trespond_data = prespond_data;
	while(trespond_data != NULL)
	{
		if(!strcmp(trespond_data->sn, dev_sn))
		{
			sn_t gw_sn = {0};
			incode_xtocs(gw_sn, get_gateway_info()->gw_no, sizeof(zidentify_no_t));

			bzero(trespond_data->dev_data, JSON_FIELD_DATA_MAXSIZE);
			STRS_MEMCPY(trespond_data->dev_data,
								pdev_info->data,
								sizeof(trespond_data->dev_data),
								strlen(pdev_info->data));

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

	if(pdev_info->ischange)
	{
		upload_data(isrefresh, NULL);
	}
}

void upload_data(uint8 isrefresh, char *random)
{
		sn_t gwno_str = {0};
		incode_xtocs(gwno_str, get_gateway_info()->gw_no, sizeof(zidentify_no_t));

		trfield_device_t **devices = NULL;
		int dev_size = 0;

		dev_info_t *p_dev = get_gateway_info()->p_dev;
		while(p_dev != NULL)
		{
			if(p_dev->ischange)
			{
				char *name = get_mix_name(p_dev->type,
											get_gateway_info()->gw_no[7],
											p_dev->dev_no[2]);

				sn_t dev_sn = {0};
				incode_xtocs(dev_sn, p_dev->dev_no, sizeof(zidentify_no_t));

				char *dev_data = (char *)calloc(1, sizeof(p_dev->data));
				memcpy(dev_data, p_dev->data, sizeof(p_dev->data));

				trfield_device_t *device = 
						get_trfield_device_alloc(name, dev_sn, (char *)p_dev->type, 1, dev_data);

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

		trfr_report_t * report = get_trfr_report_alloc(NULL,
														gwno_str,
														devices,
														dev_size,
														random);

		trans_send_report_request(get_transtocol_frhandler_arg(), report);
		get_trfr_report_free(report);
}

void device_ctrl(sn_t sn, char *cmd, char *random, respond_request_t callback)
{
	if(sn == NULL || cmd == NULL)
	{
		return;
	}


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

	//if(znet_addr)
	{
		/*int size = FR_DE_DATA_FIX_LEN + cmd_len;
		uint8 *buffer = (uint8 *)calloc(size, sizeof(uint8));

		memcpy(buffer, FR_HEAD_DE, 2);
		memcpy(buffer+2, FR_CMD_SINGLE_EXCUTE, 4);
		sprintf((char *)(buffer+6), "%04X", znet_addr);
		memcpy(buffer+10, cmd, cmd_len);
		memcpy(buffer+10+cmd_len, FR_TAIL, 4);

		serial_write((char *)buffer, size);
		free(buffer);*/
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

void trans_send_frame_request(frhandler_arg_t *arg, trans_action_t action, char *frame)
{
#ifdef TRANS_HTTP_REQUEST
		sprintf(curl_buf, "key=[%s]&datatype=%s\0", frame, get_action_to_str(action));
		curl_http_request(CURL_POST, curl_buf, curl_data);
		set_heartbeat_check(0, TRANS_WS_TIMEOUT);
#endif
}

#ifdef __cplusplus
}
#endif


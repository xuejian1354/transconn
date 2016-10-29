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

char *gen_current_checkcode()
{
	char *text = NULL;
	long text_len = 0;

	dev_info_t *p_dev = get_gateway_info()->p_dev;
	while(p_dev != NULL)
	{
		unsigned char devdata_str[8] = {0};
		incode_xtocs(devdata_str, p_dev->data, sizeof(p_dev->data));
		p_dev = p_dev->next;

		if(text == NULL)
		{
			text = (char *)calloc(1, 9);
			text_len = 9;
		}
		else
		{
			text = realloc(text, text_len+8);
			text_len += 8;
		}

		memcpy(text+text_len-9, devdata_str, 8);
		*(text+text_len-1) = '\0';
	}

	char *checkcode = get_md5(text, 16);
	free(text);

	return checkcode;
}


void trans_send_tocolreq_request(trfr_tocolreq_t *tocolreq)
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

void trans_send_report_request(trfr_report_t *report)
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

		cJSON_AddStringToObject(pDev, JSON_FIELD_DEVSN, (*(report->devices+i))->dev_sn);
		cJSON_AddStringToObject(pDev, JSON_FIELD_DEVDATA, (*(report->devices+i))->dev_data);

		i++;
	}

	cJSON_AddStringToObject(pRoot, JSON_FIELD_RANDOM, report->random);

	trans_send_frame_request(cJSON_Print(pRoot));

	cJSON_Delete(pRoot);
}

void trans_send_check_request(trfr_check_t *check)
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

	trans_send_frame_request(cJSON_Print(pRoot));

	cJSON_Delete(pRoot);
}

void trans_send_respond_request(trfr_respond_t *respond)
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

	trans_send_frame_request(cJSON_Print(pRoot));

	cJSON_Delete(pRoot);
}

void trans_refresh_handler(trfr_refresh_t *refresh)
{
	if(refresh == NULL)
	{
		return;
	}

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

	upload_data(refresh->random);
}

void trans_control_handler(trfr_control_t *control)
{
	if(control == NULL)
	{
		return;
	}

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
}


void trans_tocolres_handler(trfr_tocolres_t *tocolres)
{
	if(tocolres == NULL)
	{
		return;
	}

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
}

void trans_send_tocolres_request(trfr_tocolres_t *tocolres)
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

	trans_send_frame_request(cJSON_Print(pRoot));
	
	cJSON_Delete(pRoot);
}

void upload_data(char *random)
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
			sn_t dev_sn = {0};
			incode_xtocs(dev_sn, p_dev->dev_no, sizeof(zidentify_no_t));

			char dev_data[12] = {0};
			incode_xtocs(dev_data, p_dev->data, sizeof(p_dev->data));

			trfield_device_t *device = get_trfield_device_alloc(dev_sn, dev_data);
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
		p_dev->ischange = 0;
		p_dev = p_dev->next;
	}

	trfr_report_t * report = get_trfr_report_alloc(NULL, gwno_str, devices, dev_size, random);

	trans_send_report_request(report);
	if(report != NULL)
	{
		set_upcheck_reset();
	}

	get_trfr_report_free(report);
}

void device_ctrl(sn_t sn, char *cmd, char *random, respond_request_t callback)
{
	if(sn == NULL || cmd == NULL)
	{
		return;
	}


	/*int timer_id = (ZDEVICE_RESPOND_EVENT<<16)+atoi(get_md5(get_system_time(), 2));

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
	set_mevent(timer_id, device_ctrl_timer_callback, &param);*/

	zidentify_no_t ztsn = {0};
	incode_ctoxs(ztsn, sn, strlen(sn));

	uint8 ztcmd[8] = {0};
	uint16 ztcmdlen = strlen(cmd)/2;
	incode_ctoxs(ztcmd, cmd, strlen(cmd));


	trbuffer_t *frame = calloc(1, sizeof(trbuffer_t));
	
	frame->len = 8;
	frame->data = calloc(1, frame->len);

	*(frame->data) = ztsn[strlen(sn)/2-1];
	*(frame->data+1) = 0x04;
	if(ztcmdlen > 0)
	{
		*(frame->data+2) = ztcmd[0];
	}

	if(ztcmdlen > 1)
	{
		*(frame->data+3) = ztcmd[1];
	}

	if(ztcmdlen > 2)
	{
		*(frame->data+4) = ztcmd[2];
	}

	if(ztcmdlen > 3)
	{
		*(frame->data+5) = ztcmd[3];
	}

	uint32 crcval = crc16(frame->data, 6);
	*(frame->data+6) = (crcval>>8) & 0xFF;
	*(frame->data+7) = crcval & 0xFF;

	serial_write(frame->data, frame->len);
	PRINT_HEX(frame->data, frame->len);
	get_devopt_frame_free(frame);
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
		trans_send_tocolres_request(tocolres);
		get_trfr_tocolres_free(tocolres);

		del_respond_data_with_sn(mrespond_data->sn);
	}
}

void trans_send_frame_request(char *frame)
{
#ifdef TRANS_WS_CONNECT
	ws_send(frame, strlen(frame));
#endif
}

#ifdef __cplusplus
}
#endif


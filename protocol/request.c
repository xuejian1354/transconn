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
#include <cJSON.h>
#include <protocol/devices.h>

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
	sql_add_zdev(get_gateway_info(), pdev_info);
}
#endif


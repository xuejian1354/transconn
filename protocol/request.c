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

#ifdef COMM_CLIENT
void trans_refresh_handler(frhandler_arg_t *arg, trfr_refresh_t *refresh)
{}

void trans_control_handler(frhandler_arg_t *arg, trfr_control_t *control)
{}
#endif

void trans_protocol_request_handler(frhandler_arg_t *arg, trfr_tocolreq_t *tocolreq)
{
	if(tocolreq == NULL)
	{
		return;
	}

#ifdef COMM_CLIENT
	if(tocolreq->action == ACTION_TOCOLRES)
	{
		if(!strncmp(tocolreq->protocol, JSON_VAL_TOCOL_UDP, 3))
		{
			set_trans_protocol(TOCOL_UDP);
		}
		else if(!strncmp(tocolreq->protocol, JSON_VAL_TOCOL_TCP, 3))
		{
			set_trans_protocol(TOCOL_TCP);
		}
		else if(!strncmp(tocolreq->protocol, JSON_VAL_TOCOL_HTTP, 4))
		{
			set_trans_protocol(TOCOL_HTTP);
		}
	}
#endif
#ifdef COMM_SERVER
	if(tocolreq->action == ACTION_TOCOLREQ)
	{
		if(!strncmp(tocolreq->protocol, JSON_VAL_TOCOL_UDP, 3))
		{
			trans_send_protocol_request(arg, TOCOL_UDP);
		}
		else if(!strncmp(tocolreq->protocol, JSON_VAL_TOCOL_TCP, 3))
		{
			trans_send_protocol_request(arg, TOCOL_TCP);
		}
	}
#endif
}

void trans_send_protocol_request(frhandler_arg_t *arg, transtocol_t tocol)
{
	switch(tocol)
	{
#ifdef TRANS_UDP_SERVICE	
	case TOCOL_UDP:
	{
		cJSON *pRoot = cJSON_CreateObject();
		char action_str[4] = {0};
#ifdef COMM_SERVER
		sprintf(action_str, "%d", ACTION_TOCOLRES);
#endif
#ifdef COMM_CLIENT
		sprintf(action_str, "%d", ACTION_TOCOLREQ);
#endif
		cJSON_AddStringToObject(pRoot, JSON_FIELD_ACTION, action_str);
		cJSON_AddStringToObject(pRoot, JSON_FIELD_PROTOCOL, JSON_VAL_TOCOL_UDP);
		char *frame = cJSON_Print(pRoot);
		socket_udp_sendto(&(arg->addr), frame, strlen(frame));
		cJSON_Delete(pRoot);
	}
		break;
#endif

#if defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)	
	case TOCOL_TCP:
	{
		cJSON *pRoot = cJSON_CreateObject();
		char action_str[4] = {0};
#ifdef COMM_SERVER
		sprintf(action_str, "%d", ACTION_TOCOLRES);
#endif
#ifdef COMM_CLIENT
		sprintf(action_str, "%d", ACTION_TOCOLREQ);
#endif
		cJSON_AddStringToObject(pRoot, JSON_FIELD_ACTION, action_str);
		cJSON_AddStringToObject(pRoot, JSON_FIELD_PROTOCOL, JSON_VAL_TOCOL_TCP);
		char *frame = cJSON_Print(pRoot);
#ifdef TRANS_TCP_SERVER
		socket_tcp_server_send(arg, frame, strlen(frame));
#endif
#ifdef TRANS_TCP_CLIENT
		socket_tcp_client_send(frame, strlen(frame));
#endif
		cJSON_Delete(pRoot);
	}
		break;
#endif

#ifdef TRANS_HTTP_REQUEST
	case TOCOL_HTTP:
	{
		cJSON *pRoot = cJSON_CreateObject();
		char action_str[4] = {0};
#ifdef COMM_SERVER
		sprintf(action_str, "%d", ACTION_TOCOLRES);
#endif
#ifdef COMM_CLIENT
		sprintf(action_str, "%d", ACTION_TOCOLREQ);
#endif
		cJSON_AddStringToObject(pRoot, JSON_FIELD_ACTION, action_str);
		cJSON_AddStringToObject(pRoot, JSON_FIELD_PROTOCOL, JSON_VAL_TOCOL_HTTP);
		char *frame = cJSON_Print(pRoot);
		curl_http_request(CURL_POST, get_global_conf()->http_url, frame, curl_data);
		cJSON_Delete(pRoot);
	}
		break;
#endif
	}
}

#ifdef COMM_SERVER
void trans_report_handler(frhandler_arg_t *arg, trfr_report_t *report)
{}

void trans_check_handler(frhandler_arg_t *arg, trfr_check_t *check)
{}

void trans_respond_handler(frhandler_arg_t *arg, trfr_respond_t *respond)
{}
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

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
#include <protocol/common/mevent.h>

#ifdef COMM_CLIENT
void trans_send_tocolreq_request(frhandler_arg_t *arg, trfr_tocolreq_t *tocolreq)
{
	if(tocolreq == NULL)
	{
		return;
	}

	cJSON *pRoot = cJSON_CreateObject();
	char action_str[4] = {0};
	sprintf(action_str, "%d", ACTION_TOCOLREQ);
	cJSON_AddStringToObject(pRoot, JSON_FIELD_ACTION, action_str);

#ifdef TRANS_UDP_SERVICE	
	if(!strcmp(tocolreq->protocol, TRANSTOCOL_UDP))
	{
		cJSON_AddStringToObject(pRoot, JSON_FIELD_PROTOCOL, tocolreq->protocol);
		char *frame = cJSON_Print(pRoot);
		socket_udp_sendto(&(arg->addr), frame, strlen(frame));
	}
#endif

#ifdef TRANS_TCP_CLIENT
	if(!strcmp(tocolreq->protocol, TRANSTOCOL_TCP))
	{
		cJSON_AddStringToObject(pRoot, JSON_FIELD_PROTOCOL, tocolreq->protocol);
		char *frame = cJSON_Print(pRoot);
		socket_tcp_client_send(frame, strlen(frame));
	}
#endif

#ifdef TRANS_HTTP_REQUEST
	if(!strcmp(tocolreq->protocol, TRANSTOCOL_HTTP))
	{
		cJSON_AddStringToObject(pRoot, JSON_FIELD_PROTOCOL, tocolreq->protocol);
		char *frame = cJSON_Print(pRoot);
		curl_http_request(CURL_POST, get_global_conf()->http_url, frame, curl_data);
	}
#endif

	cJSON_Delete(pRoot);
}

void trans_send_report_request(frhandler_arg_t *arg, trfr_report_t *report)
{}

void trans_send_check_request(frhandler_arg_t *arg, trfr_check_t *check)
{
	if(check == NULL)
	{
		return;
	}

	char *frame;
	cJSON *pRoot = cJSON_CreateObject();

	char action_str[4] = {0};
	sprintf(action_str, "%d", check->action);
	cJSON_AddStringToObject(pRoot, JSON_FIELD_ACTION, action_str);
	cJSON_AddStringToObject(pRoot, JSON_FIELD_GWSN, check->gw_sn);

	cJSON *pCode = cJSON_CreateObject();
	cJSON_AddItemToObject(pRoot, JSON_FIELD_CODE, pCode);

	cJSON_AddStringToObject(pCode, JSON_FIELD_CODECHECK, check->code.code_check);
	cJSON_AddStringToObject(pCode, JSON_FIELD_CODEDATA, check->code.code_data);

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

	if(!strcmp(tocolres->protocol, TRANSTOCOL_UDP))
	{
		set_trans_protocol(TOCOL_UDP);
	}
	else if(!strcmp(tocolres->protocol, TRANSTOCOL_TCP))
	{
		set_trans_protocol(TOCOL_TCP);
	}
	else if(!strcmp(tocolres->protocol, TRANSTOCOL_HTTP))
	{
		set_trans_protocol(TOCOL_HTTP);
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
		trfr_tocolres_t *tocolres = get_trfr_tocolres_alloc(tocolreq->protocol);
		trans_send_tocolres_request(arg, tocolres);
		get_trfr_tocolres_free(tocolres);
	}
}

void trans_report_handler(frhandler_arg_t *arg, trfr_report_t *report)
{}

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

	char transtocol_str[8] = {0};
	switch(arg->transtocol)
	{
	case TOCOL_UDP:
		strcpy(transtocol_str, TRANSTOCOL_UDP);
		break;

	case TOCOL_TCP:
		strcpy(transtocol_str, TRANSTOCOL_TCP);
		break;

	case TOCOL_HTTP:
		strcpy(transtocol_str, TRANSTOCOL_HTTP);
		break;
	}

	trfr_tocolres_t *tocolres = get_trfr_tocolres_alloc(transtocol_str);
	trans_send_tocolres_request(arg, tocolres);
	get_trfr_tocolres_free(tocolres);
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

	cJSON *pRoot = cJSON_CreateObject();
	char action_str[4] = {0};
	sprintf(action_str, "%d", ACTION_TOCOLRES);
	cJSON_AddStringToObject(pRoot, JSON_FIELD_ACTION, action_str);

#ifdef TRANS_UDP_SERVICE	
	if(!strcmp(tocolres->protocol, TRANSTOCOL_UDP))
	{

		cJSON_AddStringToObject(pRoot, JSON_FIELD_PROTOCOL, tocolres->protocol);
		char *frame = cJSON_Print(pRoot);
		socket_udp_sendto(&(arg->addr), frame, strlen(frame));
	}
#endif

#if defined(TRANS_TCP_SERVER)
	if(!strcmp(tocolres->protocol, TRANSTOCOL_TCP))
	{
		cJSON_AddStringToObject(pRoot, JSON_FIELD_PROTOCOL, tocolres->protocol);
		char *frame = cJSON_Print(pRoot);
		socket_tcp_server_send(arg, frame, strlen(frame));
	}
#endif
	
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


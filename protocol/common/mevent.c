/*
 * mevent.c
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

#include "mevent.h"
#include <protocol/protocol.h>
#include <protocol/request.h>
#include <module/serial.h>
#ifdef COMM_CLIENT
#include <services/balancer.h>
#include <module/dbclient.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TIMER_SUPPORT
#ifdef COMM_CLIENT
static int heartbeat_interval = 500;

static void heartbeat_request(void *p);
static void gateway_refresh(void *p);
static void upload_refresh(void *p);
static void zdev_watch(void *p);
static void cli_watch(void *p);

void gateway_init()
{
	set_refresh_check();

	timer_event_param_t timer_param;
	timer_param.resident = 1;
	timer_param.interval = 4000;
	timer_param.count = 1;
	timer_param.immediate = 1;

	set_mevent(GATEWAY_HEARTBEAT_EVENT, heartbeat_request, &timer_param);
}

void gateway_refresh(void *p)
{
	serial_write((char *)"D:/BR/0000:O\r\n", 14);

	set_trans_protocol(TOCOL_NONE);
}

void upload_refresh(void *p)
{
	sn_t gw_sn = {0};
	incode_xtocs(gw_sn, get_gateway_info()->gw_no, sizeof(zidentify_no_t));
	if(!strcmp(gw_sn, (char *)"0000000000000000"))
	{
		serial_write((char *)"D:/BR/0000:O\r\n", 14);
		return;
	}

#ifdef TRANS_UDP_SERVICE
	frhandler_arg_t arg;
	arg.addr.sin_family = PF_INET;
	arg.addr.sin_port = htons(get_udp_port());
	arg.addr.sin_addr.s_addr = inet_addr(get_server_ip());
	trfr_tocolreq_t *utocolreq = get_trfr_tocolreq_alloc(NULL, gw_sn, TRANSTOCOL_UDP, NULL);
	trans_send_tocolreq_request(&arg, utocolreq);
	get_trfr_tocolreq_free(utocolreq);
#endif
#ifdef TRANS_TCP_CLIENT
	trfr_tocolreq_t *ttocolreq = get_trfr_tocolreq_alloc(NULL, gw_sn, TRANSTOCOL_TCP, NULL);
	trans_send_tocolreq_request(NULL, ttocolreq);
	get_trfr_tocolreq_free(ttocolreq);
#endif
#ifdef TRANS_HTTP_REQUEST
	trfr_tocolreq_t *htocolreq = get_trfr_tocolreq_alloc(NULL, gw_sn, (char *)TRANSTOCOL_HTTP, NULL);
	trans_send_tocolreq_request(NULL, htocolreq);
	get_trfr_tocolreq_free(htocolreq);
#endif
}

void set_heartbeat_check(int immediate, int interval)
{
	timer_event_param_t timer_param;
	heartbeat_interval = interval;
	timer_param.resident = 1;
	timer_param.interval = heartbeat_interval;
	timer_param.count = 1;
	timer_param.immediate = immediate;

	set_mevent(GATEWAY_HEARTBEAT_EVENT, heartbeat_request, &timer_param);
}

void set_refresh_check()
{
	timer_event_param_t timer_param;
	
	timer_param.resident = 1;
	timer_param.interval = 2*heartbeat_interval+1000;
	timer_param.count = 1;
	timer_param.immediate = 0;
	timer_param.arg = NULL;

	set_mevent(TIMER_REFRESH_EVENT, upload_refresh, &timer_param);
}

void heartbeat_request(void *p)
{
	switch(get_session_status())
	{
	case SESS_INIT:
		//Restart daemon
		//DE_PRINTF(1, "Session Init ....\n");
		break;

	case SESS_READY:
		//Refresh service
		//DE_PRINTF(1, "Session Ready ....\n");
		gateway_refresh(NULL);
		break;

	case SESS_WORKING:
	{
		//DE_PRINTF(1, "Session Working ....\n");
		upload_data(0, NULL);
	}
		break;

	case SESS_UNWORK:
		//Reset to ready status
		//DE_PRINTF(1, "Session UnWork ....\n");
		set_session_status(SESS_READY);
		break;

	case SESS_CONFIGRING:
		//Waiting cmd configuration
		//DE_PRINTF(1, "Session Configurating ....\n");
		break;

	case SESS_REALESING:
		//Sorry, I forgot
		break;
	}
}

void set_zdev_check(uint16 net_addr)
{
	timer_event_param_t timer_param;

	timer_param.resident = 0;
	timer_param.interval = 40000;
	timer_param.count = 1;
	timer_param.immediate = 0;
	timer_param.arg = (void *)((int)net_addr);
	
	set_mevent((ZDEVICE_WATCH_EVENT<<16)+net_addr, zdev_watch, &timer_param);
}

void zdev_watch(void *p)
{
	uint16 znet_addr = (uint16)((int)p);
	DE_PRINTF(1, "del zdevice from list, zdev no:%04X\n\n", znet_addr);

	sqlclient_uponline_zdev(get_gateway_info(),
							0,
							&znet_addr,
							1);

	del_zdevice_info(znet_addr);
	upload_data(0, NULL);
}

void set_cli_check(cli_info_t *p_cli)
{
	if(p_cli == NULL)
	{
		return;
	}
	
	timer_event_param_t timer_param;

	timer_param.resident = 0;
	timer_param.interval = 17000;
	timer_param.count = 1;
	timer_param.immediate = 0;
	timer_param.arg = (void *)p_cli;

	set_mevent((CLIENT_WATCH_EVENT<<16)+gen_rand(p_cli->cidentify_no), 
		cli_watch, &timer_param);
}

void cli_watch(void *p)
{
	cli_info_t *p_cli = (cli_info_t *)p;

	char clino[18] = {0};
	incode_xtocs(clino, p_cli->cidentify_no, sizeof(cidentify_no_t));
	DE_PRINTF(1, "del client from list, cli no:%s\n\n", clino);
	
	del_client_info(p_cli->cidentify_no);
}
#endif

void set_mevent(int id, timer_callback_t event_callback, timer_event_param_t *param)
{
	timer_event_t *timer_event = (timer_event_t *)calloc(1, sizeof(timer_event_t));
	timer_event->timer_id = id;
	timer_event->param = *param;
	timer_event->timer_callback = event_callback;
	
	if(set_timer_event(timer_event) != 0)
	{
		free(timer_event);
	}
}
#endif

#ifdef __cplusplus
}
#endif

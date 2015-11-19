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

#ifdef COMM_CLIENT
#define GATEWAY_INIT_EVENT		0x0001
#define TIMER_UPLOAD_EVENT		0x0002
#define ZDEVICE_WATCH_EVENT		0x0003
#define CLIENT_WATCH_EVENT		0x0006
#endif

#ifdef TIMER_SUPPORT
#ifdef COMM_CLIENT
void gateway_refresh(void *p);
void upload_event(void *p);
void zdev_watch(void *p);
void cli_watch(void *p);
#endif

#ifdef COMM_CLIENT
void gateway_init()
{
	timer_event_param_t timer_param;

	timer_param.resident = 0;
	timer_param.interval = 1;
	timer_param.count = 1;
	timer_param.immediate = 1;
	timer_param.arg = NULL;
	
	set_mevent(GATEWAY_INIT_EVENT, gateway_refresh, &timer_param);
}

void gateway_refresh(void *p)
{
	serial_write("D:/BR/0000:O\r\n", 14);

	timer_event_param_t timer_param;
	
	timer_param.resident = 1;
	timer_param.interval = 10;
	timer_param.count = 0;
	timer_param.immediate = 0;
	timer_param.arg = NULL;
	
	set_mevent(TIMER_UPLOAD_EVENT, upload_event, &timer_param);
}

void upload_event(void *p)
{	

}

void zdev_watch(void *p)
{
	uint16 znet_addr = (uint16)((int)p);
	DE_PRINTF(1, "del zdevice from list, zdev no:%04X\n\n", znet_addr);
	
	del_zdevice_info(znet_addr);
}

void cli_watch(void *p)
{
	cli_info_t *p_cli = (cli_info_t *)p;

	char clino[18] = {0};
	incode_xtocs(clino, p_cli->cidentify_no, sizeof(cidentify_no_t));
	DE_PRINTF(1, "del client from list, cli no:%s\n\n", clino);
	
	del_client_info(p_cli->cidentify_no);
}

void set_zdev_check(uint16 net_addr)
{
	timer_event_param_t timer_param;

	timer_param.resident = 0;
	timer_param.interval = 40;
	timer_param.count = 1;
	timer_param.immediate = 0;
	timer_param.arg = (void *)((int)net_addr);
	
	set_mevent((ZDEVICE_WATCH_EVENT<<16)+net_addr, zdev_watch, &timer_param);
}

void set_cli_check(cli_info_t *p_cli)
{
	if(p_cli == NULL)
	{
		return;
	}
	
	timer_event_param_t timer_param;

	timer_param.resident = 0;
	timer_param.interval = 17;
	timer_param.count = 1;
	timer_param.immediate = 0;
	timer_param.arg = (void *)p_cli;

	set_mevent((CLIENT_WATCH_EVENT<<16)+gen_rand(p_cli->cidentify_no), 
		cli_watch, &timer_param);
}
#endif

#ifdef COMM_SERVER
void gateway_watch(void *p)
{
	char gwno[18] = {0};
	incode_xtocs(gwno, p, sizeof(zidentify_no_t));
	DE_PRINTF(1, "del gateway from list, gw no:%s\n\n", gwno);

#ifdef DB_API_SUPPORT
	gw_info_t *p_gw = query_gateway_info(p);
	if(p_gw != NULL)
	{
		int offline_nums = 0;
		uint16 offline_addrs[ZDEVICE_MAX_NUM] = {0};
	
		dev_info_t *p_dev = p_gw->p_dev;
		while(p_dev != NULL)
		{
			offline_addrs[offline_nums++] = p_dev->znet_addr;
			p_dev = p_dev->next;
		}

		sql_uponline_zdev(p_gw, 0, offline_addrs, offline_nums);
	}
#endif

	del_gateway_info(p);
}

void set_gateway_check(zidentify_no_t gw_no, int rand)
{
	timer_event_param_t timer_param;

	timer_param.resident = 0;
	timer_param.interval = 24;
	timer_param.count = 1;
	timer_param.immediate = 0;
	timer_param.arg = (void *)gw_no;
	
	set_mevent(rand, gateway_watch, &timer_param);
}
#endif
void set_mevent(int id, timer_callback_t event_callback, timer_event_param_t *param)
{
	timer_event_t *timer_event = calloc(1, sizeof(timer_event_t));
	timer_event->timer_id = id;
	timer_event->param = *param;
	timer_event->timer_callback = event_callback;
	
	if(set_timer_event(timer_event) != 0)
	{
		free(timer_event);
	}
}
#endif

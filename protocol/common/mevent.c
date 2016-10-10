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

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TIMER_SUPPORT
#ifdef COMM_TARGET
static int heartbeat_interval = 500;

static void heartbeat_request(void *p);
static void zdev_watch(void *p);

void gateway_init()
{
	timer_event_param_t timer_param;
	timer_param.resident = 1;
	timer_param.interval = 4000;
	timer_param.count = 1;
	timer_param.immediate = 1;

	set_mevent(GATEWAY_HEARTBEAT_EVENT, heartbeat_request, &timer_param);
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

void heartbeat_request(void *p)
{
	
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
	//uint16 znet_addr = (uint16)((int)p);
	//DE_PRINTF(1, "del zdevice from list, zdev no:%04X\n\n", znet_addr);


	//del_zdevice_info(znet_addr);
	upload_data(0, NULL);
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

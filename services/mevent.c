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
#include <protocol/trframelysis.h>
#include <protocol/trrequest.h>

#ifdef COMM_CLIENT
#define GATEWAY_REFRESH_EVENT	0x0001
#define TIMER_UPLOAD_EVENT		0x0002
#define ZDEVICE_WATCH_EVENT		0x0003
#endif


#ifdef COMM_SERVER
#define SERVER_LISTEN_CLIENTS	0x0010
#define GATEWAY_WATCH_EVENT		0x0020
#endif

#ifdef TIMER_SUPPORT
#ifdef COMM_CLIENT
void *gateway_refresh(void *p);
void *upload_event(void *p);
void *zdev_watch(void *p);
#endif

#ifdef COMM_CLIENT
void *gateway_refresh(void *p)
{
	serial_write("D:/BR/0000:O\r\n", 14);

	timer_event_param_t timer_param;
	
	timer_param.resident = 1;
	timer_param.interval = 10;
	timer_param.count = 0;
	timer_param.immediate = 0;
	timer_param.arg = NULL;
	
	set_mevent(TIMER_UPLOAD_EVENT, upload_event, &timer_param);
	
	return NULL;
}

void *upload_event(void *p)
{
	char ipaddr[24] = {0};
	GET_SERVER_IP(ipaddr);

	send_pi_udp_request(ipaddr, TRFRAME_CON, NULL, 0, NULL);
	
	return NULL;
}

void *zdev_watch(void *p)
{
	uint16 znet_addr = (uint16)((int)p);
	del_zdevice_info(znet_addr);
}

void set_upload_event()
{
	timer_event_param_t timer_param;

	//refresh and get gateway info
	timer_param.resident = 0;
	timer_param.interval = 1;
	timer_param.count = 1;
	timer_param.immediate = 1;
	timer_param.arg = NULL;
	
	set_mevent(GATEWAY_REFRESH_EVENT, gateway_refresh, &timer_param);
}

void set_zdev_check(uint16 net_addr)
{
	timer_event_param_t timer_param;

	timer_param.resident = 0;
	timer_param.interval = 35;
	timer_param.count = 1;
	timer_param.immediate = 0;
	timer_param.arg = (void *)((int)net_addr);
	
	set_mevent((ZDEVICE_WATCH_EVENT<<16)+net_addr, zdev_watch, &timer_param);
}
#endif

#ifdef COMM_SERVER
void *gateway_watch(void *p)
{
	printf("del gateway from list\n");
	del_gateway_info(p);
}

void set_clients_listen()
{
}

void set_gateway_check(zidentify_no_t gw_no, int rand)
{
	timer_event_param_t timer_param;

	timer_param.resident = 0;
	timer_param.interval = 15;
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

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
#define CLIENT_STAND_EVENT		0x0004
#define RP_CHECK_EVENT			0x0005
#define CLIENT_WATCH_EVENT		0x0006
#endif


#ifdef COMM_SERVER
#define SERVER_LISTEN_CLIENTS	0x0010
#define GATEWAY_WATCH_EVENT		0x0020
#endif

#ifdef TIMER_SUPPORT
#ifdef COMM_CLIENT
void *gateway_refresh(void *p);
void *upload_event(void *p);
void *stand_event(void *p);
void *zdev_watch(void *p);
void *cli_watch(void *p);
void *rp_watch(void *p);
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

	timer_param.interval = 11;
	set_mevent(CLIENT_STAND_EVENT, stand_event, &timer_param);
	
	return NULL;
}

void *upload_event(void *p)
{
	char ipaddr[24] = {0};
	GET_SERVER_IP(ipaddr);
	
	dev_info_t *p_dev = get_gateway_info()->p_dev;
	char buffer[ZDEVICE_MAX_NUM<<2] = {0};
	char bsize = 0;
	while(p_dev != NULL && bsize < (ZDEVICE_MAX_NUM<<2))
	{
		incode_xtoc16(buffer+bsize, p_dev->znet_addr);
		bsize += 4;
		p_dev = p_dev->next;
	}

	send_pi_udp_request(ipaddr, TRFRAME_CON, buffer, bsize, NULL);
	
	return NULL;
}

void *stand_event(void *p)
{
 	cli_info_t *p_cli = get_client_list()->p_cli;

	dev_info_t *p_dev = get_gateway_info()->p_dev;
	char buffer[ZDEVICE_MAX_NUM<<4] = {0};
	char bsize = 0;
	while(p_dev != NULL && bsize < (ZDEVICE_MAX_NUM<<4))
	{
		incode_xtocs(buffer+bsize, p_dev->zidentity_no, 8);
		bsize += 16;
		p_dev = p_dev->next;
	}

	while(p_cli != NULL)
	{
		send_gd_udp_request(p_cli->ipaddr, TRINFO_REG, 
			get_gateway_info()->gw_no, p_cli->cidentify_no, buffer, bsize);

		if(p_cli->check_conn)
		{
			set_cli_check(p_cli);
			p_cli->check_conn = 0;
		}
		
		p_cli = p_cli->next;
	}
	
	return NULL;
}

void *zdev_watch(void *p)
{
	uint16 znet_addr = (uint16)((int)p);
	del_zdevice_info(znet_addr);
}

void *cli_watch(void *p)
{
	cli_info_t *p_cli = (cli_info_t *)p;
	del_client_info(p_cli->cidentify_no);
}


void *rp_watch(void *p)
{
	cli_info_t *p_cli = (cli_info_t *)p;
	if(p_cli == NULL)
	{
		return;
	}
	
	if(p_cli->check_count-- != 0)
	{
		send_rp_udp_respond(p_cli->ipaddr, TRINFO_UPDATE, 
			get_gateway_info()->gw_no, p_cli->cidentify_no, NULL, 0);
		
		set_rp_check(p_cli);
	}
	else
	{
		char ipaddr[24] = {0};
		GET_SERVER_IP(ipaddr);

		send_rp_udp_respond(ipaddr, TRINFO_IP, get_gateway_info()->gw_no, 
			p_cli->cidentify_no, p_cli->ipaddr, p_cli->ip_len);

		memset(p_cli->ipaddr, 0, sizeof(p_cli->ipaddr));
		memcpy(p_cli->ipaddr, ipaddr, strlen(ipaddr));
		p_cli->ip_len = strlen(ipaddr);
	}
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

void set_cli_check(cli_info_t *p_cli)
{
	if(p_cli == NULL)
	{
		return;
	}
	
	timer_event_param_t timer_param;

	timer_param.resident = 0;
	timer_param.interval = 15;
	timer_param.count = 1;
	timer_param.immediate = 0;
	timer_param.arg = (void *)p_cli;

	set_mevent((CLIENT_WATCH_EVENT<<16)+gen_rand(p_cli->cidentify_no), 
		cli_watch, &timer_param);
}

void set_rp_check(cli_info_t *p_cli)
{
	timer_event_param_t timer_param;

	timer_param.resident = 0;
	timer_param.interval = 1;
	timer_param.count = 1;
	timer_param.immediate = 0;
	timer_param.arg = (void *)p_cli;

	if(p_cli == NULL)
	{
		timer_param.interval = 0;
		timer_param.count = 0;
	}
	
	set_mevent(RP_CHECK_EVENT, rp_watch, &timer_param);
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

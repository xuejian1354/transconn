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
#include <protocol/request.h>

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
void *gateway_refresh(void *p, pthread_mutex_t *lock);
void *upload_event(void *p, pthread_mutex_t *lock);
void *stand_event(void *p, pthread_mutex_t *lock);
void *zdev_watch(void *p, pthread_mutex_t *lock);
void *cli_watch(void *p, pthread_mutex_t *lock);
void *rp_watch(void *p, pthread_mutex_t *lock);
#endif

#ifdef COMM_CLIENT
void *gateway_refresh(void *p, pthread_mutex_t *lock)
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

void *upload_event(void *p, pthread_mutex_t *lock)
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
	
	pi_t pi;
	memcpy(pi.sn, get_gateway_info()->gw_no, sizeof(zidentify_no_t));
	pi.trans_type = TRTYPE_UDP_NORMAL;
	pi.fr_type = TRFRAME_CON;
	pi.data = buffer;
	pi.data_len = bsize;
	
	send_frame_udp_request(ipaddr, TRHEAD_PI, &pi);
	
	return NULL;
}

void *stand_event(void *p, pthread_mutex_t *lock)
{
 	cli_info_t *p_cli = get_client_list()->p_cli;

	dev_info_t *p_dev = get_gateway_info()->p_dev;
	char buffer[ZDEVICE_MAX_NUM<<2] = {0};
	char bsize = 0;
	while(p_dev != NULL && bsize < (ZDEVICE_MAX_NUM<<2))
	{
		incode_xtoc16(buffer+bsize, p_dev->znet_addr);
		bsize += 4;
		p_dev = p_dev->next;
	}

	char ipaddr[24] = {0};
	GET_SERVER_IP(ipaddr);

	while(p_cli != NULL)
	{
		if(memcmp(ipaddr, p_cli->ipaddr, p_cli->ip_len))
		{
			gd_t gd;
			memcpy(gd.zidentify_no, get_gateway_info()->gw_no, sizeof(zidentify_no_t));
			memcpy(gd.cidentify_no, p_cli->cidentify_no, sizeof(cidentify_no_t));
			gd.trans_type = TRTYPE_UDP_TRAVERSAL;
			gd.tr_info = TRINFO_REG;
			gd.data = buffer;
			gd.data_len = bsize;
			send_frame_udp_request(p_cli->ipaddr, TRHEAD_GD, &gd);
		}

		if(p_cli->check_conn)
		{
			set_cli_check(p_cli);
			p_cli->check_conn = 0;
		}
		
		p_cli = p_cli->next;
	}
	
	return NULL;
}

void *zdev_watch(void *p, pthread_mutex_t *lock)
{
	uint16 znet_addr = (uint16)((int)p);
	DE_PRINTF(1, "del zdevice from list, zdev no:%04X\n\n", znet_addr);
	
	del_zdevice_info(znet_addr);
}

void *cli_watch(void *p, pthread_mutex_t *lock)
{
	cli_info_t *p_cli = (cli_info_t *)p;

	char clino[18] = {0};
	incode_xtocs(clino, p_cli->cidentify_no, sizeof(cidentify_no_t));
	DE_PRINTF(1, "del client from list, cli no:%s\n\n", clino);
	
	del_client_info(p_cli->cidentify_no);
}


void *rp_watch(void *p, pthread_mutex_t *lock)
{
	cli_info_t *p_cli = (cli_info_t *)p;
	if(p_cli == NULL)
	{
		return;
	}
	
	if(p_cli->check_count-- != 0)
	{
		rp_t rp;
		memcpy(rp.zidentify_no, get_gateway_info()->gw_no, sizeof(zidentify_no_t));
		memcpy(rp.cidentify_no, p_cli->cidentify_no, sizeof(cidentify_no_t));
		rp.trans_type = TRTYPE_UDP_TRAVERSAL;
		rp.tr_info = TRINFO_UPDATE;
		rp.data = NULL;
		rp.data_len = 0;
		enable_datalog_atime();
		send_frame_udp_request(p_cli->ipaddr, TRHEAD_RP, &rp);
		
		set_rp_check(p_cli);
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
void *gateway_watch(void *p, pthread_mutex_t *lock)
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

void set_clients_listen()
{
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

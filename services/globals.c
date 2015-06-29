/* globals.c
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
#include "globals.h"
#include <pthread.h>
#include <errno.h>
#include <services/mevent.h>
#include <protocol/protocol.h>

#ifdef COMM_CLIENT
static char serial_port[16] = "/dev/ttyS1";
static int tcp_port = TRANS_TCP_PORT;
static int udp_port = TRANS_UDP_REMOTE_PORT;
#endif

#ifdef COMM_SERVER
static int tcp_port = TRANS_TCP_PORT;
static int udp_port = TRANS_UDP_PORT;
#endif

#ifdef COMM_CLIENT
char *get_serial_port()
{
	return serial_port;
}

void set_serial_port(char *name)
{
	memset(serial_port, 0, sizeof(serial_port));
	strcpy(serial_port, name);
}
#endif

#if defined(COMM_CLIENT) || defined(COMM_SERVER)
int get_tcp_port()
{
	return tcp_port;
}

int get_udp_port()
{
	return udp_port;
}

void set_tcp_port(int port)
{
	tcp_port = port;
}

void set_udp_port(int port)
{
	udp_port = port;
}
#endif

int mach_init()
{
#ifdef COMM_CLIENT
	gw_info_t *p_gw_info = get_gateway_info();

	memset(p_gw_info->gw_no, 0, sizeof(p_gw_info->gw_no));
	p_gw_info->zapp_type = FRAPP_NONE;
	p_gw_info->zpanid = 0;
	p_gw_info->zchannel = 0;
	p_gw_info->ip_len = 0;
	p_gw_info->zgw_opt = NULL;
	p_gw_info->p_dev = NULL;
	p_gw_info->next = NULL;
	
	if(pthread_mutex_init(&(get_gateway_info()->lock), NULL) != 0)
    {
        fprintf(stderr, "%s: pthread_mutext_init failed, errno:%d, error:%s\n",
            __FUNCTION__, errno, strerror(errno));
        return -1;
    }

	cli_list_t *p_cli_list = get_client_list();
	p_cli_list->p_cli = NULL;
	p_cli_list->max_num = 0;

	if(pthread_mutex_init(&(get_client_list()->lock), NULL) != 0)
    {
        fprintf(stderr, "%s: pthread_mutext_init failed, errno:%d, error:%s\n",
            __FUNCTION__, errno, strerror(errno));
        return -1;
    }
#endif

#ifdef COMM_SERVER
	gw_list_t *p_gw_list = get_gateway_list();
	p_gw_list->p_gw = NULL;
	p_gw_list->max_num = 0;

	if(pthread_mutex_init(&(get_gateway_list()->lock), NULL) != 0)
    {
        fprintf(stderr, "%s: pthread_mutext_init failed, errno:%d, error:%s\n",
            __FUNCTION__, errno, strerror(errno));
        return -1;
    }
#endif

	return 0;
}

void event_init()
{
#ifdef TIMER_SUPPORT
#ifdef COMM_CLIENT
	set_upload_event();
#endif

#ifdef COMM_SERVER
	set_clients_listen();
#endif
#endif
}
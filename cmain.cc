/*
 * cmain.cc
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
#include <services/globals.h>
#include <services/balancer.h>
#include <services/etimer.h>
#include <services/corecomm.h>
#include <module/serial.h>
#include <module/netapi.h>
#ifdef DB_API_SUPPORT
#include <module/dbclient.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

int main(int argc, char **argv)
{
#ifdef LOAD_BALANCE_SUPPORT
	if(serlist_read_from_confile() < 0)
	{
		DE_PRINTF(1, "%s()%d : Read \"%s\" fail, please set correct server list file\n",
			__FUNCTION__, __LINE__, 
			BALANCE_SERVER_FILE);
		
		return -1;
	}
#endif

#ifdef READ_CONF_FILE
	if(conf_read_from_file() < 0)
	{
		return -1;
	}
#endif
	
	if(start_params(argc, argv) != 0)
	{
		return 1;
	}

#ifdef DAEMON_PROCESS_CREATE
	signal(SIGCHLD, SIG_IGN);
    if(daemon_init() != 0)
    {
		return 0;
	}
#endif

#ifdef DB_API_SUPPORT
	if(sqlclient_init() < 0)
	{
		sqlclient_release();
		return -1;
	}
	//sqlclient_release();
#endif

#ifdef THREAD_POOL_SUPPORT
	if (tpool_create(TRANS_CLIENT_THREAD_MAX_NUM) < 0)
	{
		return -1;
	}
#endif

#ifdef TIMER_SUPPORT
	if(timer_init() < 0)
	{
		return -1;
	}
#endif

#ifdef SELECT_SUPPORT
	if(select_init() < 0)
	{
		return -1;
	}
#endif

#ifdef SERIAL_SUPPORT
	if(serial_init(get_serial_dev()) < 0)			//init serial device
	{
		return -1;
	}
#endif

#if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG)
	if (socket_udp_service_init(get_udp_port()) < 0)
	{
		return -1;
	}
#endif

#ifdef TRANS_TCP_CLIENT
	if (socket_tcp_client_connect(get_tcp_port()) < 0)
	{
		return -1;
	}
#endif

#ifdef TRANS_WS_CONNECT
	if(ws_init(get_global_conf()->ws_url) < 0)
	{
		return -1;
	}
#endif

	if(mach_init() < 0)
	{
		return -1;
	}

	event_init();

	while(1)
	{
#ifdef SELECT_SUPPORT
		select_listen();
#else
		usleep(10000);
#endif
	}

	return 0;
}

#ifdef __cplusplus
}
#endif

/*
 * smain.c
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
#include <module/netapi.h>
#ifdef DB_API_SUPPORT
#include <module/dbserver.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

int main(int argc, char **argv)
{
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
	if(sqlserver_init() < 0)
	{
		sqlserver_release();
		return -1;
	}
	//sqlserver_release();
#endif

#ifdef THREAD_POOL_SUPPORT
	if (tpool_create(TRANS_SERVER_THREAD_MAX_NUM) < 0)
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
	select_init();
#endif

#if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG) ||  defined(DE_TRANS_UDP_CONTROL)
	if (socket_udp_service_init(get_udp_port()) < 0)
	{
		return -1;
	}
#endif

#ifdef TRANS_TCP_SERVER
	if (socket_tcp_server_init(get_tcp_port()) < 0)
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
#endif
	}
	
	return 0;
}

#ifdef __cplusplus
}
#endif

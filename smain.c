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
#include <module/balancer.h>

int main(int argc, char **argv)
{
#ifdef LOAD_BALANCE_SUPPORT
	serlist_read_from_confile();
#endif
	
	int tcp_port = TRANS_TCP_PORT;
	int udp_port = TRANS_UDP_PORT;

	START_PARAMS(tcp_port, udp_port);

#ifdef THREAD_POOL_SUPPORT
	if (tpool_create(TRANS_THREAD_MAX_NUM) < 0)
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

#ifdef TRANS_UDP_SERVICE
	if (socket_udp_service_init(udp_port) < 0)
	{
		return -1;
	}
#endif

#ifdef TRANS_TCP_SERVER
	if (socket_tcp_server_init(tcp_port) < 0)
	{
		return -1;
	}
#endif

	if(mach_init() < 0)
	{
		return -1;
	}
	
	event_init();

#ifdef DEBUG_CONSOLE_SUPPORT
	create_console();
#endif

	while(1)
	{
#ifdef SELECT_SUPPORT
		select_listen();
#endif
	}

end:	
	printf("end!(#>_<#)\n");
	return 0;
}

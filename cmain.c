/*
 * cmain.c
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

int main(int argc, char **argv)
{
#ifdef LOAD_BALANCE_SUPPORT
	if(serlist_read_from_confile() < 0)
	{
		printf("%s()%d : Read \"%s\" fail, please set correct server list file\n",
			__FUNCTION__, __LINE__, 
			BALANCE_SERVER_FILE);
		
		return -1;
	}
#endif
	
	if(start_params(argc, argv) != 0)
	{
		return 1;
	}

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

#ifdef SERIAL_SUPPORT
	if(serial_init(get_serial_port()) < 0)			//initial serial port
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

#ifdef TRANS_UDP_SERVICE
	if (socket_udp_service_init(get_udp_port()) < 0)
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


/*
 * Becareful!!! This function eat all CPU, Default no used.
*/
#ifdef TRANS_TCP_CLIENT
	if (socket_tcp_client_init() < 0)
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
	printf("%s()%d : end!(#>_<#)\n", __FUNCTION__, __LINE__);
	return 0;
}


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
#include <mconfig.h>

int main(int argc, char **argv)
{
	char serial_port[16] = "/dev/ttyS1";
	int udp_port = TRANS_UDP_REMOTE_PORT;
	
	START_PARAMS(serial_port, udp_port);

#ifdef TIMER_SUPPORT
	if(timer_initial() < 0)
	{
		return -1;
	}
#endif

#ifdef THREAD_POOL_SUPPORT
	if (tpool_create(TRANS_THREAD_MAX_NUM) < 0)
	{
		return -1;
	}
#endif

#ifdef SERIAL_SUPPORT
	if(serial_initial(serial_port) < 0)			//initial serial port
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
	if (socket_udp_service_init() < 0)
	{
		return -1;
	}
#endif

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


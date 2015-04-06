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
	char serial_port[16] = "/dev/ttyUSB0";
	int udp_port = TRANS_UDP_REMOTE_PORT;

	if(argc > 1)
	{	
		memset(serial_port, 0, sizeof(serial_port));
		strcpy(serial_port, argv[1]);
	}

	if(argc > 2)
		udp_port = atoi(argv[2]);

	if(argc > 3)
	{
		DE_PRINTF("Usage: %s [Serial Port] [Server Port]\n", TARGET_NAME);
		return -1;
	}

	DE_PRINTF("%s start!\n", TARGET_NAME);
	DE_PRINTF("Serial Port:%s, UDP Port:%d\n", serial_port, udp_port);

#ifdef SERIAL_SUPPORT
	if(serial_initial(serial_port) < 0)			//initial serial port
	{
		return -1;
	}
#endif

	select_init();

#ifdef THREAD_POOL_SUPPORT
	if (tpool_create(TRANS_THREAD_MAX_NUM) < 0)
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
		select_listen();
	}

end:	
	printf("end!(#>_<#)\n");
	return 0;
}


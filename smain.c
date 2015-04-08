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
#include <mconfig.h>

int main(int argc, char **argv)
{
	int tcp_port = TRANS_TCP_PORT;
	int udp_port = TRANS_UDP_PORT;

	unsigned char buf[MAXSIZE];

#ifdef SELECT_SUPPORT
	select_init();
#endif

	if (argc > 1)
		tcp_port = atoi(argv[1]);

	if (argc > 2)
		udp_port = atoi(argv[2]);

	if (argc > 3)
	{
		DE_PRINTF("Usage:%s [TCP Port] [UDP Port]\n", TARGET_NAME);
		return -1;
	}

#ifdef TRANS_TCP_SERVER
	if (socket_tcp_server_init(tcp_port) < 0)
	{
		return -1;
	}
#endif

#ifdef TRANS_UDP_SERVICE
	if (socket_udp_service_init(udp_port) < 0)
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

	DE_PRINTF("%s start!\n", TARGET_NAME);
	DE_PRINTF("TCP Port:%d, UDP Port:%d\n", tcp_port, udp_port);

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

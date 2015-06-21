/*
 * dlog.h
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
#ifndef __DLOG_H__
#define __DLOG_H__

#include <debug/dconfig.h>

#define DLOG_FILE   "/tmp/trans_log.txt"

#define DE_PRINTF(args...)  printf(args);

/*#define   DE_PRINTF(args...)  \
st( \
    FILE *fp = NULL;    \
    if((fp = fopen(DLOG_FILE, "a+")) != NULL)   \
    {   \
        fprintf(fp, args);  \
        fclose(fp); \
    }   \
)*/

#ifdef COMM_CLIENT
#define START_PARAMS(serial_port, tcp_port, udp_port)								\
st(																					\
	if(argc > 1)																	\
	{																				\
		memset(serial_port, 0, sizeof(serial_port));								\
		strcpy(serial_port, argv[1]);												\
	}																				\
																					\
	if (argc > 2)																	\
		tcp_port = atoi(argv[1]);													\
																					\
	if(argc > 3)																	\
		udp_port = atoi(argv[2]);													\
																					\
	if(argc > 4)																	\
	{																				\
		DE_PRINTF("Usage: %s [Serial Port] [TCP Port] [UDP Port]\n", TARGET_NAME);	\
		return -1;																	\
	}																				\
																					\
	DE_PRINTF("%s start!\n", TARGET_NAME);											\
	DE_PRINTF("Server:%s\nIP:%s\n", 												\
		get_server_name_from_ip(get_server_ip()), get_server_ip());					\
	DE_PRINTF("Serial Port:%s, TCP Port:%d, UDP Port:%d\n", 						\
		serial_port, tcp_port, udp_port);											\
)

#elif defined(COMM_SERVER)
#define START_PARAMS(tcp_port, udp_port)											\
st(																					\
	if (argc > 1)																	\
		tcp_port = atoi(argv[1]);													\
																					\
	if (argc > 2)																	\
		udp_port = atoi(argv[2]);													\
																					\
	if (argc > 3)																	\
	{																				\
		DE_PRINTF("Usage:%s [TCP Port] [UDP Port]\n", TARGET_NAME);					\
		return -1;																	\
	}																				\
																					\
	DE_PRINTF("%s start!\n", TARGET_NAME);											\
	DE_PRINTF("Server:%s\nIP:%s\n", 												\
		get_server_name_from_ip((char *)get_server_ip()), get_server_ip());					\
	DE_PRINTF("TCP Port:%d, UDP Port:%d\n", tcp_port, udp_port);					\
)

#else
#define START_PARAMS(...)
#endif

#define PRINT_HEX(data, len)			\
st(										\
	int x;								\
	for(x=0; x<len; x++)				\
	{									\
		DE_PRINTF("%02X ", data[x]);	\
	}									\
	DE_PRINTF("\n");					\
)

#endif  //__DLOG_H__

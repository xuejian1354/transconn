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

//#define DE_TRANS_UDP_STREAM_LOG

//#define DLOG_FILE   "/tmp/trans_log.txt"

#ifdef DE_TRANS_UDP_STREAM_LOG
#define DE_PRINTF(args...)  \
st(  \
	char buf[1024] = {0};  \
	sprintf(buf, args);  \
	delog_udp_sendto(buf, strlen(buf));	  \
)
#else
#define DE_PRINTF(args...)	printf(args)
#endif

/*#define   DE_PRINTF(args...)  \
st( \
    FILE *fp = NULL;    \
    if((fp = fopen(DLOG_FILE, "a+")) != NULL)   \
    {   \
        fprintf(fp, args);  \
        fclose(fp); \
    }   \
)*/


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

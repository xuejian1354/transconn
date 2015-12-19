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

#ifdef __cplusplus
extern "C" {
#endif

#ifdef COMM_CLIENT
#define DLOG_FILE   "/var/log/connect_listen.log"
#elif defined(COMM_SERVER)
#define DLOG_FILE   "/var/log/transconn.log"
#else
#define DLOG_FILE   "/tmp/transconn.log"
#endif

#ifdef DE_TRANS_UDP_STREAM_LOG
#define DE_PRINTF(lwflag, format, args...)  \
st(  \
	FILE *fp = NULL;    \
	char *buf = get_de_buf();  \
	sprintf(buf, format, ##args);  \
	delog_udp_sendto(buf, strlen(buf));	  \
    if(lwflag && (fp = fopen(DLOG_FILE, "a+")) != NULL)   \
    {   \
		fprintf(fp, get_time_head());    \
        fprintf(fp, format, ##args);  \
        fclose(fp); \
    }   \
)
#else
#define DE_PRINTF(lwflag, format, args...)	   \
st(    \
	FILE *fp = NULL;    \
	printf(format, ##args);    \
	if(lwflag && (fp = fopen(DLOG_FILE, "a+")) != NULL)    \
    {    \
		fprintf(fp, get_time_head());    \
        fprintf(fp, format, ##args);    \
        fclose(fp);    \
    }    \
)
#endif

#define PRINT_HEX(data, len)			\
st(										\
	int x;								\
	for(x=0; x<len; x++)				\
	{									\
		DE_PRINTF(0, "%02X ", data[x]);	\
	}									\
	DE_PRINTF(0, "\n");					\
)

#ifdef __cplusplus
}
#endif

#endif  //__DLOG_H__

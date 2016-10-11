/*
 * mconfig.h
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
#ifndef __MCONFIG_H__
#define __MCONFIG_H__

#include <mtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

//#define COMM_TARGET

#define THREAD_POOL_SUPPORT
#define TIMER_SUPPORT
#define SELECT_SUPPORT
#define UART_COMMBY_SOCKET

#define SERIAL_SUPPORT
//#define TRANS_TCP_SERVER
//#define TRANS_TCP_CLIENT
//#define TRANS_UDP_SERVICE
#define TRANS_HTTP_REQUEST
#define TRANS_WS_CONNECT
//#define DAEMON_PROCESS_CREATE
//#define SERIAL_SUPPORT
#define READ_CONF_FILE
//#define DE_TRANS_UDP_STREAM_LOG

#ifdef DE_TRANS_UDP_STREAM_LOG
#define DAEMON_PROCESS_CREATE
#endif

#if defined(TRANS_TCP_SERVER) && defined(TRANS_TCP_CLIENT)
#error 'cannot define TRANS_TCP_SERVER and TRANS_TCP_CLIENT at the same time'
#endif

#ifdef READ_CONF_FILE
#define CONF_FILE	"/etc/transconn.conf"
#endif

#ifdef __cplusplus
}
#endif

#endif //__MCONFIG_H__


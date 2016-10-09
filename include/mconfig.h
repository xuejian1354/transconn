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

#if defined(COMM_TARGET)
#define THREAD_POOL_SUPPORT
#define TIMER_SUPPORT
#define SELECT_SUPPORT
#ifdef COMM_TARGET
#define UART_COMMBY_SOCKET
#endif
#endif
//#define TRANS_TCP_SERVER
//#define TRANS_TCP_CLIENT
//#define TRANS_UDP_SERVICE
//#define TRANS_HTTP_REQUEST
//#define TRANS_WS_CONNECT
//#define DAEMON_PROCESS_CREATE
//#define SERIAL_SUPPORT
//#define READ_CONF_FILE
//#define DB_API_SUPPORT
//#define DE_TRANS_UDP_STREAM_LOG
//#define DE_TRANS_UDP_CONTROL

#ifdef DB_API_SUPPORT
#ifdef COMM_TARGET
#define DB_API_WITH_SQLITE
#endif
#endif

#if defined(DB_API_WITH_MYSQL) && defined(DB_API_WITH_SQLITE)
#error "cannot define DB_API_WITH_MYSQL with DB_API_WITH_SQLITE at same time"
#endif

#ifdef DE_TRANS_UDP_STREAM_LOG
#define DAEMON_PROCESS_CREATE
#endif

#define BIND_SUPERBUTTON_CTRL_SUPPORT

#if defined(TRANS_TCP_SERVER) && defined(TRANS_TCP_CLIENT)
#error 'cannot define TRANS_TCP_SERVER and TRANS_TCP_CLIENT at the same time'
#endif

#define BALANCE_SERVER_FILE		"/etc/balance_serlist"

#ifdef READ_CONF_FILE
#define CONF_FILE	"/etc/transconn.conf"
#elif !defined(COMM_TARGET)
  #ifdef DB_API_SUPPORT
    #error 'You must define READ_CONF_FILE first before defining DB_API_SUPPORT'
  #elif defined(REMOTE_UPDATE_APK)
    #error 'You must define READ_CONF_FILE first before defining REMOTE_UPDATE_APK'
  #endif
#endif

#if !defined(TRANS_UDP_SERVICE) && defined(REMOTE_UPDATE_APK)
#error 'You must define TRANS_UDP_SERVICE first before defining REMOTE_UPDATE_APK'
#endif

#ifdef __cplusplus
}
#endif

#endif //__MCONFIG_H__


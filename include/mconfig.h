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

//#define COMM_SERVER
//#define COMM_CLIENT

//#define TRANS_TCP_SERVER
//#define TRANS_UDP_SERVICE
//#define THREAD_POOL_SUPPORT
//#define TIMER_SUPPORT
//#define SERIAL_SUPPORT
//#define SELECT_SUPPORT
//#define DEBUG_CONSOLE_SUPPORT

#define BIND_SUPERBUTTON_CTRL_SUPPORT

#ifdef LOAD_BALANCE_SUPPORT
#define BALANCE_SERVER_FILE		"/etc/balance_serlist"
#endif

#ifdef READ_CONF_FILE
#define CONF_FILE	"/etc/transconn.conf"
#endif

#endif //__MCONFIG_H__


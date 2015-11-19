/*
 * dconfig.h
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
#ifndef __DCONFIG_H__
#define __DCONFIG_H__

#include <mconfig.h>

#ifdef COMM_SERVER
#define DE_PRINT_UDP_PORT
#define DE_PRINT_TCP_PORT
#endif

#ifdef COMM_CLIENT
#define DE_PRINT_UDP_PORT
#define DE_PRINT_TCP_PORT
#define DE_PRINT_SERIAL_PORT

//#define DE_ZDEVICE_RECORD
#define RECORD_FILE		"/tmp/transconn_record.txt"
#endif
#endif //__DCONFIG_H__


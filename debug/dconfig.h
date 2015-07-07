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

#define DE_PRINT_UDP_PI		0x01
#define DE_PRINT_UDP_BI		0x02
#define DE_PRINT_UDP_GP		0x04
#define DE_PRINT_UDP_RP		0x08
#define DE_PRINT_UDP_GD		0x10
#define DE_PRINT_UDP_RD		0x20
#define DE_PRINT_UDP_DC		0x40
#define DE_PRINT_UDP_UB		0x80

#ifdef COMM_SERVER
#define DE_PRINT_UDP_PORT	0xFF
#define DE_PRINT_TCP_PORT
#endif

#ifdef COMM_CLIENT
#define DE_PRINT_UDP_PORT	0xFF
#define DE_PRINT_TCP_PORT
#define DE_PRINT_SERIAL_PORT

//#define DE_ZDEVICE_RECORD
#define RECORD_FILE		"/tmp/transconn_record.txt"
#define TCP_CONN_TMP	"/tmp/transconn_mtmp.txt"
#endif
#endif //__DCONFIG_H__


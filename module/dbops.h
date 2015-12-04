/*
 * dbops.h
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
#ifndef __DBOPS_H__
#define __DBOPS_H__

#ifdef DB_API_SUPPORT

#include <services/globals.h>
#include <protocol/protocol.h>
#include <strings_t.h>
#ifdef DB_API_WITH_MYSQL
#include <mysql/mysql.h>
#endif

#define CMDLINE_SIZE	0x4000
#define GET_CMD_LINE()	cmdline

#define SET_CMD_LINE(format, args...)  	\
st(  									\
	bzero(cmdline, sizeof(cmdline));  	\
	sprintf(cmdline, format, ##args);  	\
)


#endif

#endif	//__DBOPS_H__
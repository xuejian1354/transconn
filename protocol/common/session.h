/*
 * session.h
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
#ifndef __SESSION_H__
#define __SESSION_H__

#include <services/globals.h>

#define TRANSTOCOL_HTTP		"http"
#define TRANSTOCOL_TCP		"tcp"
#define TRANSTOCOL_UDP		"udp"

typedef enum
{
	SESS_INIT,
	SESS_READY,
	SESS_WORKING,
	SESS_UNWORK,
	SESS_CONFIGRING,
	SESS_REALESING
}sessionsta_t;

typedef enum
{
	TOCOL_DISABLE = 0x0000,
	TOCOL_ENABLE = 0x0001,
	TOCOL_UDP = 0x0002,
	TOCOL_TCP = 0x0004,
	TOCOL_HTTP = 0x0008,
	TOCOL_NONE = 0x0010,
}transtocol_t;

void set_session_status(sessionsta_t session);
sessionsta_t get_session_status();

void set_trans_protocol(transtocol_t tocol);
transtocol_t get_trans_protocol();

#endif  //__SESSION_H__

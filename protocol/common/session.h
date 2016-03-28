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

#ifdef __cplusplus
extern "C" {
#endif

#define TRANSTOCOL_WS		"websocket"
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
	TOCOL_WS = 0x0010,
	TOCOL_NONE = 0x0020,
}transtocol_t;

void set_session_status(sessionsta_t session);
sessionsta_t get_session_status();

void set_trans_protocol(uint16 tocol);
uint16 get_trans_protocol();
char *get_trans_protocol_to_str(uint16 tocol);

void set_syncdata_checkcode(char *code);
char *get_syncdata_checkcode();
char *gen_current_checkcode(zidentify_no_t gw_sn);

#ifdef __cplusplus
}
#endif

#endif  //__SESSION_H__

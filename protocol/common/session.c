/*
 * session.c
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
#include "session.h"
#include <protocol/request.h>
#include <module/netapi.h>
#include <services/balancer.h>

static sessionsta_t session_status;
static transtocol_t transtocol;

#ifdef COMM_CLIENT
void send_transtocol_request();
#ifdef TRANS_UDP_SERVICE
void transtocol_udp_respond_handler(char *data);
#endif
#ifdef TRANS_TCP_CLIENT
void transtocol_tcp_respond_handler(char *data);
#endif
#ifdef TRANS_HTTP_REQUEST
size_t transtocol_post_respond_handler(void *buffer, 
					size_t size, size_t nmemb, void *userp);
#endif
#endif

void set_session_status(sessionsta_t session)
{
	session_status = session;
}

sessionsta_t get_session_status()
{
	return session_status;
}

void set_trans_protocol(transtocol_t tocol)
{
	transtocol = tocol;
}
transtocol_t get_trans_protocol()
{
	return transtocol;
}

void session_handler()
{
	switch(session_status)
	{
	case SESS_INIT:
		break;

	case SESS_READY:
#ifdef COMM_CLIENT
		send_transtocol_request();
#endif
		break;

	case SESS_WORKING:
		break;

	case SESS_UNWORK:
		break;

	case SESS_CONFIGRING:
		break;

	case SESS_REALESING:
		break;

	default:
		break;
	}
}

#ifdef COMM_CLIENT
void send_transtocol_request()
{}

#ifdef TRANS_UDP_SERVICE
void transtocol_udp_respond_handler(char *data)
{}
#endif

#ifdef TRANS_TCP_CLIENT
void transtocol_tcp_respond_handler(char *data)
{}
#endif

#ifdef TRANS_HTTP_REQUEST
size_t transtocol_post_respond_handler(void *buffer, 
					size_t size, size_t nmemb, void *userp)
{
	return size*nmemb;
}
#endif
#endif

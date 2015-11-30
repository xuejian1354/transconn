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
#include <sqlite3.h>
#include <protocol/request.h>
#include <protocol/common/mevent.h>
#include <module/netapi.h>
#include <module/dbopt.h>
#include <services/balancer.h>

extern char cmdline[CMDLINE_SIZE];

static sessionsta_t session_status;
static transtocol_t transtocol;
static char checkcode[64];

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
	switch(tocol)
	{
	case TOCOL_DISABLE:
		transtocol &= ~TOCOL_ENABLE;
		break;
		
	case TOCOL_ENABLE:
		transtocol |= TOCOL_ENABLE;
		break;
		
	case TOCOL_UDP:
	case TOCOL_TCP:
	case TOCOL_HTTP:
	{
#ifdef COMM_SERVER
		int i = 0;
		while(i < get_global_conf()->tocol_len)
		{
			transtocol_t enable_tocol = get_global_conf()->protocols[i];
			if(enable_tocol & tocol)
			{
				transtocol |= tocol;
				set_session_status(SESS_WORKING);
				break;
			}

			i++;
		}
#endif
#ifdef COMM_CLIENT
		transtocol |= tocol;
		set_session_status(SESS_WORKING);
#endif
	}
		break;

	case TOCOL_NONE:
#ifdef COMM_CLIENT
		transtocol &= ~(TOCOL_UDP | TOCOL_TCP | TOCOL_HTTP);
		set_session_status(SESS_WORKING);
#endif
		break;
	}
}
transtocol_t get_trans_protocol()
{
#ifdef COMM_CLIENT
	if(!(transtocol & TOCOL_ENABLE))
	{
		return TOCOL_DISABLE;
	}

	int i = 0;
	while(i < get_global_conf()->tocol_len)
	{
		transtocol_t enable_tocol = get_global_conf()->protocols[i];
		if(enable_tocol & transtocol)
		{
			return enable_tocol;
		}

		i++;
	}
	
	return TOCOL_NONE;
#endif

#ifdef COMM_SERVER
	return transtocol;
#endif
}

void set_syncdata_checkcode(char *code)
{
	bzero(checkcode, sizeof(checkcode));
	STRS_MEMCPY(checkcode, code, sizeof(checkcode), strlen(code));
}

char *get_syncdata_checkcode()
{
	return checkcode;
}

char *gen_current_checkcode(zidentify_no_t gw_sn)
{
	gw_info_t *p_gw = NULL;
#ifdef COMM_CLIENT
	p_gw = get_gateway_info();
#elif defined(COMM_SERVER)
	p_gw = query_gateway_info(gw_sn);
#endif

	char *text = NULL;
	long text_len = 0;

	if(p_gw != NULL)
	{
#ifdef DB_API_WITH_SQLITE
		char gwsn[20] = {0};
		incode_xtocs(gwsn, p_gw->gw_no, sizeof(zidentify_no_t));
		SET_CMD_LINE("%s%s%s",
						"SELECT data,isonline FROM devices WHERE gwsn=\'",
						gwsn,
						"\'");

	 	sqlite3_stmt *stmt;
	    if(sqlite3_prepare_v2(get_sqlite_db(), GET_CMD_LINE(), -1, &stmt, NULL) == SQLITE_OK)
	    {		
	        while(sqlite3_step(stmt) == SQLITE_ROW)
			{
				if(sqlite3_column_count(stmt) > 1)
				{
					const char *data = sqlite3_column_text(stmt, 0);
					const char *isonline = sqlite3_column_text(stmt, 1);
					if(data == NULL || isonline == NULL)
					{
						continue;
					}

					int data_len = strlen(data);

					if(text == NULL)
					{
						text = calloc(1, data_len+2);
						text_len = data_len + 2;
					}
					else
					{
						text = realloc(text, text_len+data_len+1);
						text_len += data_len+1;
					}

					memcpy(text+text_len-data_len-2, data, data_len);
					*(text+text_len-2) = *isonline;
					*(text+text_len-1) = '\0';
				}
			}
	    }
		sqlite3_finalize(stmt);
#else
		dev_info_t *p_dev = p_gw->p_dev;
		while(p_dev != NULL)
		{
			fr_buffer_t *buffer = get_devopt_data_tostr(p_dev->zdev_opt);
			p_dev = p_dev->next;

			if(buffer == NULL)
			{
				continue;
			}

			if(text == NULL)
			{
				text = calloc(1, buffer->size+2);
				text_len = buffer->size+2;
			}
			else
			{
				text = realloc(text, text_len+buffer->size+1);
				text_len += buffer->size+1;
			}

			memcpy(text+text_len-buffer->size-2, buffer->data, buffer->size);
			*(text+text_len-2) = '1';
			*(text+text_len-1) = '\0';

			get_buffer_free(buffer);
		}
#endif
	}

	char *checkcode = get_md5(text, 16);
	free(text);

	return checkcode;
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

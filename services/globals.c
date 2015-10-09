/* globals.c
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
#include "globals.h"
#include <pthread.h>
#include <errno.h>
#include <dirent.h>
#include <services/mevent.h>
#include <protocol/protocol.h>

#ifdef COMM_CLIENT
static char serial_port[16] = "/dev/ttyS1";
static int tcp_port = TRANS_TCP_PORT;
static int udp_port = TRANS_UDP_REMOTE_PORT;
static uint8 _broadcast_no[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
#endif

#ifdef COMM_SERVER
static int tcp_port = TRANS_TCP_PORT;
static int udp_port = TRANS_UDP_PORT;
static uint8 _common_no[8] = {0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF};
#endif

#ifdef READ_CONF_FILE
global_conf_t g_conf = {0};

static void get_read_line(char *line, int len);
static void set_conf_val(char *cmd, char *val);
#endif

#ifdef COMM_CLIENT
char *get_serial_port()
{
	return serial_port;
}

void set_serial_port(char *name)
{
	memset(serial_port, 0, sizeof(serial_port));
	strcpy(serial_port, name);
}
#endif

#if defined(COMM_CLIENT) || defined(COMM_SERVER)
int get_tcp_port()
{
	return tcp_port;
}

int get_udp_port()
{
	return udp_port;
}

void set_tcp_port(int port)
{
	tcp_port = port;
}

void set_udp_port(int port)
{
	udp_port = port;
}
#endif

#ifdef COMM_CLIENT
uint8 *get_broadcast_no()
{
	return _broadcast_no;
}
#endif

#ifdef COMM_SERVER
uint8 *get_common_no()
{
	return _common_no;
}
#endif

int mach_init()
{
#ifdef COMM_CLIENT
	gw_info_t *p_gw_info = get_gateway_info();

	memset(p_gw_info->gw_no, 0, sizeof(p_gw_info->gw_no));
	p_gw_info->zapp_type = FRAPP_NONE;
	p_gw_info->zpanid = 0;
	p_gw_info->zchannel = 0;
	p_gw_info->ip_len = 0;
	p_gw_info->zgw_opt = NULL;
	p_gw_info->p_dev = NULL;
	p_gw_info->next = NULL;
	
	if(pthread_mutex_init(&(get_gateway_info()->lock), NULL) != 0)
    {
        fprintf(stderr, "%s: pthread_mutext_init failed, errno:%d, error:%s\n",
            __FUNCTION__, errno, strerror(errno));
        return -1;
    }

	cli_list_t *p_cli_list = get_client_list();
	p_cli_list->p_cli = NULL;
	p_cli_list->max_num = 0;

	if(pthread_mutex_init(&(get_client_list()->lock), NULL) != 0)
    {
        fprintf(stderr, "%s: pthread_mutext_init failed, errno:%d, error:%s\n",
            __FUNCTION__, errno, strerror(errno));
        return -1;
    }
#endif

#ifdef COMM_SERVER
	gw_list_t *p_gw_list = get_gateway_list();
	p_gw_list->p_gw = NULL;
	p_gw_list->max_num = 0;

	if(pthread_mutex_init(&(get_gateway_list()->lock), NULL) != 0)
    {
        fprintf(stderr, "%s: pthread_mutext_init failed, errno:%d, error:%s\n",
            __FUNCTION__, errno, strerror(errno));
        return -1;
    }
#endif

	return 0;
}

void event_init()
{
#ifdef TIMER_SUPPORT
#ifdef COMM_CLIENT
	set_upload_event();
#endif

#ifdef COMM_SERVER
	set_clients_listen();
#endif
#endif
}

#ifdef READ_CONF_FILE
void conf_read_from_file()
{
	FILE *fp = NULL;
	char buf[128] = {0};
		
	if((fp = fopen(CONF_FILE, "r")) != NULL)
	{
		while(fgets(buf, sizeof(buf), fp) != NULL)
		{
			get_read_line(buf, strlen(buf));	
			memset(buf, 0, sizeof(buf));
		}
	}
}

void get_read_line(char *line, int len)
{
	int i, s;
	int is_ignore = 0;

	int cmd_ph, cmd_pt, val_ph, val_pt;
	char p_isset = 0x07;
	
	if(line == NULL || len < 3)
	{
		return;
	}
	
	for(i=0; i<len; i++)
	{	
		switch(*(line+i))
		{
		case ' ':
			break;

		case '#':
			if(!is_ignore)
			{
				return;
			}
			
		default:
			is_ignore = 1;
			if(p_isset & 0x01)
			{
				cmd_ph = i;
				p_isset &= ~0x01;
			}
			else if(p_isset & 0x02)
			{
				if(*(line+i) == '=')
				{
					int j;
					for(j=i-1; j>=cmd_ph; j--)
					{
						if(*(line+j) != ' ')
						{
							cmd_pt = j+1;
							p_isset &= ~0x02;
							break;
						}
					}
				}
			}
			else if(p_isset & 0x04)
			{
				val_ph = i;
				p_isset &= ~0x04;
			}
			
			break;
		}
	}

	for(s=len-2; s>=val_ph; s--)
	{
		if(*(line+s) != ' ')
		{
			val_pt = s+1;
			break;
		}
	}

	if((cmd_ph < cmd_pt)
		&& (cmd_pt < val_ph)
		&& (val_ph < val_pt))
	{
		char cmd[64] = {0};
		char val[64] = {0};

		memcpy(cmd, line+cmd_ph, cmd_pt-cmd_ph);
		memcpy(val, line+val_ph, val_pt-val_ph);

		set_conf_val(cmd, val);
	}
}

void set_conf_val(char *cmd, char *val)
{
#ifdef REMOTE_UPDATE_APK
	if(!strcmp(cmd, GLOBAL_CONF_UPAPK_DIR))
	{
		strcpy(g_conf.upapk_dir, val);
	}
#endif
}

#ifdef REMOTE_UPDATE_APK
void reapk_version_code(char *ipaddr, cidentify_no_t cidentify_no)
{
	ub_t ub;
	DIR *dp;
	struct dirent *ep;

	int version_code = 0;
	
	memcpy(ub.zidentify_no, cidentify_no, sizeof(cidentify_no_t));
	memcpy(ub.cidentify_no, cidentify_no, sizeof(cidentify_no_t));
	ub.trans_type = TRTYPE_UDP_NORMAL;
	ub.tr_info = TRINFO_REDATA;

	if((dp = opendir(g_conf.upapk_dir)) != NULL)
	{
		while((ep = readdir(dp)) != NULL)
		{
			int f_len = strlen(ep->d_name);
			
			if(f_len > 12
				&& !strncmp(ep->d_name, "SHomely_", 8)
				&& !strncmp(ep->d_name+f_len-4, ".apk", 4))
			{
				char codestr[16] = {0};
				memcpy(codestr, ep->d_name+8, f_len-12);
				int tcode = atoi(codestr);
				version_code = version_code>tcode?version_code:tcode;
			}
		}
	}

	char data[4];
	incode_xtoc16(data, version_code);
	
	ub.data = data;
	ub.data_len = 4;
		
	send_frame_udp_request(ipaddr, TRHEAD_UB, &ub);
}
#endif
#endif


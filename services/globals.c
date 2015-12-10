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
#include <md5.h>
#include <protocol/common/fieldlysis.h>
#include <protocol/common/session.h>
#include <protocol/protocol.h>
#include <services/balancer.h>

char cmdline[CMDLINE_SIZE];

static char current_time[64];
static char curcode[64];
static uint8 _broadcast_no[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static char port_buf[16];
#ifdef COMM_CLIENT
static char serial_dev[16] = TRANS_SERIAL_DEV;
static int tcp_port = TRANS_TCP_PORT;
static int udp_port = TRANS_UDP_REMOTE_PORT;
#endif

#ifdef COMM_SERVER
static int tcp_port = TRANS_TCP_PORT;
static int udp_port = TRANS_UDP_PORT;
static uint8 _common_no[8] = {0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF};
#endif

#ifdef DE_TRANS_UDP_STREAM_LOG
static char de_buf[0x4000];
#endif

static global_conf_t g_conf = 
{
	0,
	{TOCOL_UDP, TOCOL_TCP, TOCOL_HTTP, 0},
	3,
#ifdef SERIAL_SUPPORT
	TRANS_SERIAL_DEV,
#endif
#if defined(COMM_CLIENT) && (defined(TRANS_UDP_SERVICE) || defined(TRANS_TCP_CLIENT))
	{0},
#endif
#if defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
	TRANS_TCP_PORT,
#ifdef COMM_CLIENT
	TRANS_TCP_TIMEOUT,
#endif
#endif
#if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG) || defined(DE_TRANS_UDP_CONTROL)
	TRANS_UDP_PORT,
#ifdef COMM_CLIENT
	TRANS_UDP_TIMEOUT,
#endif
#endif
#ifdef TRANS_HTTP_REQUEST
	{0},
#ifdef COMM_CLIENT
	TRANS_HTTP_TIMEOUT,
#endif
#endif
#ifdef REMOTE_UPDATE_APK
	TRANS_UPDATE_DIR,
#endif
#ifdef DB_API_SUPPORT
#if defined(DB_API_WITH_MYSQL) || defined(DB_API_WITH_SQLITE)
	TRANS_DB_NAME,
#ifdef DB_API_WITH_MYSQL
	TRANS_DB_USER,
	TRANS_DB_PASS,
#endif
#endif
#endif
};

#ifdef READ_CONF_FILE
static void get_read_line(char *line, int len);
static void set_conf_val(char *cmd, char *val);
static int get_conf_setval();
#endif

static char cur_time[64];

#ifdef DE_TRANS_UDP_STREAM_LOG
char *get_de_buf()
{
	bzero(de_buf, sizeof(de_buf));
	return de_buf;
}
#endif

#ifdef COMM_CLIENT
char *get_serial_dev()
{
	return serial_dev;
}

void set_serial_dev(char *name)
{
	memset(serial_dev, 0, sizeof(serial_dev));
	strcpy(serial_dev, name);
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

uint8 *get_broadcast_no()
{
	return _broadcast_no;
}

#ifdef COMM_SERVER
uint8 *get_common_no()
{
	return _common_no;
}
#endif

int start_params(int argc, char **argv)
{

	int ch;
	opterr = 0;  
	global_conf_t t_conf = {0};


#ifdef SERIAL_SUPPORT
  #if defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
    #if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG) || defined(DE_TRANS_UDP_CONTROL)
	char *optstrs = "s:t:u:h";
	#else
	char *optstrs = "s:t:h";
    #endif
  #elif defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG) || defined(DE_TRANS_UDP_CONTROL)
	char *optstrs = "s:u:h";
  #else
  	char *optstrs = "s:h";
  #endif
#elif defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
  #if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG) || defined(DE_TRANS_UDP_CONTROL)
  	char *optstrs = "t:u:h";
  #else
  	char *optstrs = "t:h";
  #endif
#elif defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG) || defined(DE_TRANS_UDP_CONTROL)
	char *optstrs = "u:h";
#else
	char *optstrs = "h";
#endif
	
    while((ch = getopt(argc, argv, optstrs)) != -1)
    {
		switch(ch)
		{
		case 'h':
#ifdef SERIAL_SUPPORT
  #if defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
    #if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG) || defined(DE_TRANS_UDP_CONTROL)
			DE_PRINTF(0, "Usage: %s [-s<Serial Device>] [-t<TCP Port>] [-u<UDP Port>]\n", argv[0]);
			DE_PRINTF(0, "Default: \n\t-s %s\n\t-t %d\n\t-u %d\n",
							TRANS_SERIAL_DEV,
							TRANS_TCP_PORT,
							TRANS_UDP_PORT);
	#else
			DE_PRINTF(0, "Usage: %s [-s<Serial Device>] [-t<TCP Port>]\n", argv[0]);
			DE_PRINTF(0, "Default:\n\t-s %s\n\t-t %d\n", TRANS_SERIAL_DEV, TRANS_TCP_PORT);
    #endif
  #elif defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG) || defined(DE_TRANS_UDP_CONTROL)
			DE_PRINTF(0, "Usage: %s [-s<Serial Device>] [-u<UDP Port>]\n", argv[0]);
  			DE_PRINTF(0, "Default:\n\t-s %s\n\t-u %d\n", TRANS_SERIAL_DEV, TRANS_UDP_PORT);
  #else
  			DE_PRINTF(0, "Usage: %s [-s<Serial Device>]\n", argv[0]);
  			DE_PRINTF(0, "Default:\n\t-s %s\n", TRANS_SERIAL_DEV);
  #endif
#elif defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
  #if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG) || defined(DE_TRANS_UDP_CONTROL)
  			DE_PRINTF(0, "Usage: %s [-t<TCP Port>] [-u<UDP Port>]\n", argv[0]);
  			DE_PRINTF(0, "Default:\n\t-t %d\n\t-u %d\n", TRANS_TCP_PORT, TRANS_UDP_PORT);
  #else
  			DE_PRINTF(0, "Usage: %s [-t<TCP Port>]\n", argv[0]);
  			DE_PRINTF(0, "Default:\n\t-t %d\n", TRANS_TCP_PORT);
  #endif
#elif defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG) || defined(DE_TRANS_UDP_CONTROL)
			DE_PRINTF(0, "Usage: %s [-u<UDP Port>]\n", argv[0]);
			DE_PRINTF(0, "Default:\n\t-u %d\n", TRANS_UDP_PORT);
#else
			DE_PRINTF(0, "Usage: %s\n", argv[0]);
#endif
			return 1;

		case '?':
			DE_PRINTF(0, "Unrecognize arguments.\n");
			DE_PRINTF(0, "\'%s -h\' get more help infomations.\n", argv[0]);
			return 1;

#ifdef SERIAL_SUPPORT
		case 's':
			sprintf(t_conf.serial_dev, "%s", optarg);
			t_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_SERIAL;
			break;
#endif
#if defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
		case 't':
			t_conf.tcp_port = atoi(optarg);
			t_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_TCP;
			break;
#endif
#if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG) || defined(DE_TRANS_UDP_CONTROL)
		case 'u':
			t_conf.udp_port = atoi(optarg);
			t_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_UDP;
			break;
#endif
		}
	}
	
#ifdef SERIAL_SUPPORT
	if(t_conf.isset_flag & GLOBAL_CONF_ISSETVAL_SERIAL)
	{
		set_serial_dev(t_conf.serial_dev);
	}
	else
	{
  #ifdef READ_CONF_FILE
		set_serial_dev(g_conf.serial_dev);
  #else
		set_serial_dev(TRANS_SERIAL_DEV);
  #endif
	}
	
  #if defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
  	if(t_conf.isset_flag & GLOBAL_CONF_ISSETVAL_TCP)
  	{
		set_tcp_port(t_conf.tcp_port);
  	}
	else
	{
	#ifdef READ_CONF_FILE
		set_tcp_port(g_conf.tcp_port);
    #else
		set_tcp_port(TRANS_TCP_PORT);
    #endif
	}
	
    #if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG) || defined(DE_TRANS_UDP_CONTROL)

	if(t_conf.isset_flag & GLOBAL_CONF_ISSETVAL_UDP)
	{
		set_udp_port(t_conf.udp_port);
	}
	else
	{
      #ifdef READ_CONF_FILE
		set_udp_port(g_conf.udp_port);
      #else
		set_udp_port(TRANS_UDP_PORT);
      #endif
	}
    #endif
	
  #elif defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG) || defined(DE_TRANS_UDP_CONTROL)
	if(t_conf.isset_flag & GLOBAL_CONF_ISSETVAL_UDP)
  	{
		set_udp_port(t_conf.udp_port);
  	}
	else
	{
	#ifdef READ_CONF_FILE
		set_udp_port(g_conf.udp_port);
    #else
		set_udp_port(TRANS_UDP_PORT);
    #endif
	}
  #endif

#elif defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
	if(t_conf.isset_flag & GLOBAL_CONF_ISSETVAL_TCP)
  	{
		set_tcp_port(t_conf.tcp_port);
  	}
	else
	{
  #ifdef READ_CONF_FILE
		set_tcp_port(g_conf.tcp_port);
  #else
		set_tcp_port(TRANS_TCP_PORT);
  #endif
	}

  #if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG) || defined(DE_TRANS_UDP_CONTROL)
  	if(t_conf.isset_flag & GLOBAL_CONF_ISSETVAL_UDP)
  	{
		set_udp_port(t_conf.udp_port);
  	}
	else
	{
    #ifdef READ_CONF_FILE
		set_udp_port(g_conf.udp_port);
    #else
		set_udp_port(TRANS_UDP_PORT);
    #endif
	}
  #endif
  
#elif defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG) || defined(DE_TRANS_UDP_CONTROL)
	if(t_conf.isset_flag & GLOBAL_CONF_ISSETVAL_UDP)
  	{
		set_udp_port(t_conf.udp_port);
  	}
	else
	{
    #ifdef READ_CONF_FILE
		set_udp_port(g_conf.udp_port);
    #else
		set_udp_port(TRANS_UDP_PORT);
    #endif
	}
#elif defined(TRANS_HTTP_REQUEST)	
#else
#warning "No Comm protocol be selected, please set uart, tcp or udp."
#endif

#if defined(TRANS_HTTP_REQUEST) && !defined(READ_CONF_FILE)
	sprintf(get_global_conf()->http_url, "http://%s/request", SERVER_IP);
#endif

#ifdef COMM_CLIENT
	DE_PRINTF(1, "Gateway Start!\n");
#else
	DE_PRINTF(1, "Server Start!\n");
#endif

#ifdef SERIAL_SUPPORT
	DE_PRINTF(1, "Serial device: \"%s\"\n", get_serial_dev());
#endif

#if defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
	DE_PRINTF(1, "TCP transmit port: %d\n", get_tcp_port());
#endif

#if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG) || defined(DE_TRANS_UDP_CONTROL)
	DE_PRINTF(1, "UDP transmit port: %d\n", get_udp_port());
#endif

#ifdef TRANS_HTTP_REQUEST
	DE_PRINTF(1, "HTTP URL: \"%s\"\n", get_global_conf()->http_url);
#endif

	FILE *fp = NULL;
	static char logfile[32] = {0};
    if((fp = fopen(DLOG_FILE, "a+")) != NULL)
    {
		char rebuf[1];
		if(fread(rebuf, 1, 1, fp) > 0 )
		{
			fclose(fp);
			
			time_t t;
			time(&t);
			struct tm *tp= localtime(&t);
			sprintf(logfile, "%s%s_", "/var/log/", TARGET_NAME);
			strftime(logfile+strlen(logfile), 100, "%Y-%m-%d-%H:%M:%S", tp);
			sprintf(logfile+strlen(logfile), "%s", ".log");

			char cmd[256] = {0};
			sprintf(cmd, "mv %s %s\n", DLOG_FILE, logfile);
			system(cmd);
			
			if((fp = fopen(DLOG_FILE, "w+")) == NULL)
			{
				goto openlog_error;
			}
		}

		fclose(fp);
    }
	else
	{
openlog_error:
		DE_PRINTF(1, "%s()%d : disable write log to \"%s\", please set correct user or goups permissions for dir \"/var/log\"\n", 
			__FUNCTION__, __LINE__, DLOG_FILE);
	}

	set_session_status(SESS_INIT);
	return 0;
}


char *get_time_head()
{
	time_t t;
	bzero(cur_time, sizeof(cur_time));
	time(&t);
	struct tm *tp= localtime(&t);
	strftime(cur_time, 100, "[%Y-%m-%d-%H:%M:%S] ", tp);

	return cur_time;
}

#ifdef DAEMON_PROCESS_CREATE
static int daemon_cmdline_flag = 0;

int daemon_init()
{
    int pid;
	
    if(pid = fork())
	{
		return 1;
    }
	else if(pid < 0)
	{
		DE_PRINTF(1, "%s()%d : excute failed\n", __FUNCTION__, __LINE__);
		return -1;
	}

    setsid();
	
	if(pid = fork())
	{
		return 1;
	}
    else if(pid < 0)
	{
		DE_PRINTF(1, "%s()%d : excute failed\n", __FUNCTION__, __LINE__);
		return -1;
    }

	int i;
	int fdtablesize = getdtablesize();
    for(i = 0; i < fdtablesize; i++)
    {
        close(i);
    }
	
    chdir("/tmp");
    umask(0);

	daemon_cmdline_flag = 1;
    return 0;
}

int get_daemon_cmdline()
{
	return daemon_cmdline_flag;
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
        DE_PRINTF(1, "%s()%d :  pthread_mutext_init failed, errno:%d, error:%s\n",
            __FUNCTION__, __LINE__, errno, strerror(errno));
        return -1;
    }

	cli_list_t *p_cli_list = get_client_list();
	p_cli_list->p_cli = NULL;
	p_cli_list->max_num = 0;

	if(pthread_mutex_init(&(get_client_list()->lock), NULL) != 0)
    {
        DE_PRINTF(1, "%s()%d :  pthread_mutext_init failed, errno:%d, error:%s\n",
            __FUNCTION__, __LINE__, errno, strerror(errno));
        return -1;
    }
#endif

	set_session_status(SESS_READY);
	set_trans_protocol(TOCOL_ENABLE);
	return 0;
}

void event_init()
{
#ifdef TIMER_SUPPORT
#ifdef COMM_CLIENT
	gateway_init();
#endif
#endif
}

global_conf_t *get_global_conf()
{
	return &g_conf;
}

#ifdef READ_CONF_FILE
int conf_read_from_file()
{
	FILE *fp = NULL;
	char buf[128] = {0};
	g_conf.isset_flag = 0;
	memset(g_conf.protocols, 0, sizeof(g_conf.protocols));
	g_conf.tocol_len = 0;

	if((fp = fopen(CONF_FILE, "r")) != NULL)
	{
		while(fgets(buf, sizeof(buf), fp) != NULL)
		{
			get_read_line(buf, strlen(buf));	
			memset(buf, 0, sizeof(buf));
		}
	}
	else
	{
		DE_PRINTF(1, "%s()%d :  Read \"%s\" error, please set configuration file\n", 
			__FUNCTION__, __LINE__, 
			CONF_FILE);
		return -1;
	}

	if(get_conf_setval() < 0)
	{
		return -1;
	}

	return 0;
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
	if(!strcmp(cmd, GLOBAL_CONF_COMM_PROTOCOL))
	{
		int i, len=strlen(val);
		int start_pos, end_pos;
		int start_isset = 0;
		int end_isset = 0;
		int pro_index = 0;
		transtocol_t transtocol_hasset = TOCOL_DISABLE;

		for(i=0; i<=len; i++)
		{
			if(start_isset && (end_isset || i == len))
			{
				int field_len = end_pos - start_pos;
				if(i == len && !end_isset)
				{
					field_len++;
				}

				start_isset = 0;
				end_isset = 0;

				if(!(transtocol_hasset & TOCOL_UDP) && field_len == 3
					&& !strncmp(val+start_pos, TRANSTOCOL_UDP, field_len))
				{
					g_conf.protocols[pro_index++] = TOCOL_UDP;
					transtocol_hasset |= TOCOL_UDP;
				}
				else if(!(transtocol_hasset & TOCOL_TCP) && field_len == 3
					&& !strncmp(val+start_pos, TRANSTOCOL_TCP, field_len))
				{
					g_conf.protocols[pro_index++] = TOCOL_TCP;
					transtocol_hasset |= TOCOL_TCP;
				}
				else if(!(transtocol_hasset & TOCOL_HTTP) && field_len == 4
					&& !strncmp(val+start_pos, TRANSTOCOL_HTTP, field_len))
				{
					g_conf.protocols[pro_index++] = TOCOL_HTTP;
					transtocol_hasset |= TOCOL_HTTP;
				}

				if(i == len)
				{
					break;
				}
			}

			if(!start_isset)
			{
				if(*(val+i) == ' '
					|| *(val+i) == ',')
				{
					continue;
				}
				else
				{
					start_pos = i;
					end_pos = i+1;
					start_isset = 1;
				}
			}
			else if(!end_isset)
			{
				end_pos = i;
				if(*(val+i) == ',')
				{
					int j = end_pos;
					while(j > start_pos+1)
					{
						if(*(val+j-1) == ' ')
						{
							j--;
						}
						else
						{
							break;
						}
					}
					end_pos = j;
					end_isset = 1;
				}
			}
		}

		if(pro_index)
		{
			g_conf.tocol_len = pro_index;
			g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_PROTOCOL;
		}
	}

#ifdef SERIAL_SUPPORT
	if(!strcmp(cmd, GLOBAL_CONF_SERIAL_PORT))
	{
		strcpy(g_conf.serial_dev, val);
		g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_SERIAL;
	}
#endif
#if defined(COMM_CLIENT) && (defined(TRANS_UDP_SERVICE) || defined(TRANS_TCP_CLIENT))
	if(!strcmp(cmd, GLOBAL_CONF_MAIN_IP))
	{
		confval_list *pval = get_confval_alloc_from_str(val);
		if(pval != NULL)
		{
			strcpy(g_conf.main_ip, get_val_from_name(pval->val));
			g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_IP;
			get_confval_free(pval);
		}
		else
		{
			strcpy(g_conf.main_ip, val);
			g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_IP;
		}
	}
#endif
#if defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
	if(!strcmp(cmd, GLOBAL_CONF_TCP_PORT))
	{
		confval_list *pval = get_confval_alloc_from_str(val);
		if(pval != NULL)
		{
			g_conf.tcp_port = atoi(get_val_from_name(pval->val));
			g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_TCP;
			get_confval_free(pval);
		}
		else
		{
			g_conf.tcp_port = atoi(val);
			if(g_conf.tcp_port > 0)
			{
				g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_TCP;
			}
		}
	}
#ifdef COMM_CLIENT
	if(!strcmp(cmd, GLOBAL_CONF_TCP_TIMEOUT))
	{
		g_conf.tcp_timeout = atoi(val);
		if(g_conf.tcp_timeout > 0)
		{
			g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_TCP_TIMEOUT;
		}
	}
#endif
#endif

#if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG) || defined(DE_TRANS_UDP_CONTROL)
	if(!strcmp(cmd, GLOBAL_CONF_UDP_PORT))
	{
		confval_list *pval = get_confval_alloc_from_str(val);
		if(pval != NULL)
		{
			g_conf.udp_port = atoi(get_val_from_name(pval->val));
			g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_UDP;
			get_confval_free(pval);
		}
		else
		{
			g_conf.udp_port = atoi(val);
			if(g_conf.udp_port > 0)
			{
				g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_UDP;
			}
		}
	}
#ifdef COMM_CLIENT
	if(!strcmp(cmd, GLOBAL_CONF_UDP_TIMEOUT))
	{
		g_conf.udp_timeout = atoi(val);
		if(g_conf.udp_timeout > 0)
		{
			g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_UDP_TIMEOUT;
		}
	}
#endif
#endif

#ifdef TRANS_HTTP_REQUEST
	if(!strcmp(cmd, GLOBAL_CONF_HTTP_URL))
	{		
		translate_confval_to_str(g_conf.http_url, val);

		if(strlen(g_conf.http_url))
		{
			g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_HTTPURL;
		}
	}
#ifdef COMM_CLIENT
	if(!strcmp(cmd, GLOBAL_CONF_HTTP_TIMEOUT))
	{
		g_conf.http_timeout = atoi(val);
		if(g_conf.http_timeout > 0)
		{
			g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_HTTP_TIMEOUT;
		}
	}
#endif
#endif

#ifdef REMOTE_UPDATE_APK
	if(!strcmp(cmd, GLOBAL_CONF_UPAPK_DIR))
	{
		strcpy(g_conf.upapk_dir, val);
		g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_UPAPK;
	}
#endif

#ifdef DB_API_SUPPORT
#if defined(DB_API_WITH_MYSQL) || defined(DB_API_WITH_SQLITE)
	if(!strcmp(cmd, GLOBAL_CONF_DATABASE))
	{
		strcpy(g_conf.db_name, val);
		g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_DB;
	}
#ifdef DB_API_WITH_MYSQL
	else if(!strcmp(cmd, GLOBAL_CONF_DBUSER))
	{
		strcpy(g_conf.db_user, val);
		g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_DBUSER;
	}
	else if(!strcmp(cmd, GLOBAL_CONF_DBPASS))
	{
		strcpy(g_conf.db_password, val);
		g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_DBPASS;
	}
#endif
#endif
#endif
}

int get_conf_setval()
{
	int i;
	uint32 issetflags[] = {
					GLOBAL_CONF_ISSETVAL_PROTOCOL,
#ifdef SERIAL_SUPPORT
					GLOBAL_CONF_ISSETVAL_SERIAL,
#endif
#if defined(COMM_CLIENT) && (defined(TRANS_UDP_SERVICE) || defined(TRANS_TCP_CLIENT))
					GLOBAL_CONF_ISSETVAL_IP,
#endif
#if defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
					GLOBAL_CONF_ISSETVAL_TCP,
#ifdef COMM_CLIENT
					GLOBAL_CONF_ISSETVAL_TCP_TIMEOUT,
#endif
#endif
#if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG) || defined(DE_TRANS_UDP_CONTROL)
					GLOBAL_CONF_ISSETVAL_UDP,
#ifdef COMM_CLIENT
					GLOBAL_CONF_ISSETVAL_UDP_TIMEOUT,
#endif
#endif
#ifdef TRANS_HTTP_REQUEST
					GLOBAL_CONF_ISSETVAL_HTTPURL,
#ifdef COMM_CLIENT
					GLOBAL_CONF_ISSETVAL_HTTP_TIMEOUT,
#endif
#endif
#ifdef REMOTE_UPDATE_APK
					GLOBAL_CONF_ISSETVAL_UPAPK,
#endif
#ifdef DB_API_SUPPORT
#if defined(DB_API_WITH_MYSQL) || defined(DB_API_WITH_SQLITE)
					GLOBAL_CONF_ISSETVAL_DB,
#ifdef DB_API_WITH_MYSQL
					GLOBAL_CONF_ISSETVAL_DBUSER,
					GLOBAL_CONF_ISSETVAL_DBPASS,
#endif
#endif
#endif
					};

	char *issetvals[] = {
					GLOBAL_CONF_COMM_PROTOCOL,
#ifdef SERIAL_SUPPORT
					GLOBAL_CONF_SERIAL_PORT,
#endif
#if defined(COMM_CLIENT) && (defined(TRANS_UDP_SERVICE) || defined(TRANS_TCP_CLIENT))
					GLOBAL_CONF_MAIN_IP,
#endif
#if defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
					GLOBAL_CONF_TCP_PORT,
#ifdef COMM_CLIENT
					GLOBAL_CONF_TCP_TIMEOUT,
#endif
#endif
#if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG) || defined(DE_TRANS_UDP_CONTROL)
					GLOBAL_CONF_UDP_PORT,
#ifdef COMM_CLIENT
					GLOBAL_CONF_UDP_TIMEOUT,
#endif
#endif
#ifdef TRANS_HTTP_REQUEST
					GLOBAL_CONF_HTTP_URL,
#ifdef COMM_CLIENT
					GLOBAL_CONF_HTTP_TIMEOUT,
#endif
#endif
#ifdef REMOTE_UPDATE_APK
					GLOBAL_CONF_UPAPK_DIR,
#endif
#ifdef DB_API_SUPPORT
#if defined(DB_API_WITH_MYSQL) || defined(DB_API_WITH_SQLITE)
					GLOBAL_CONF_DATABASE,
#ifdef DB_API_WITH_MYSQL
					GLOBAL_CONF_DBUSER,
					GLOBAL_CONF_DBPASS,
#endif
#endif
#endif
					};


	for(i=0; i<sizeof(issetflags)/sizeof(uint32); i++)
	{
		if(!(g_conf.isset_flag & issetflags[i]))
		{
			DE_PRINTF(1, "%s()%d : val \"%s\" is not set in \"%s\"\n",
							__FUNCTION__, __LINE__, 
							issetvals[i],
							CONF_FILE);
			
			return -1;
		}
	}
	
	return 0;
}

void translate_confval_to_str(char *dst, char *src)
{
	int i;
	int head_isset = 0;
	int head_pos = 0;
	int tail_pos = 0;
	int vallen = strlen(src);

	confval_list *pval = get_confval_alloc_from_str(src);

	for(i=0; i<vallen; i++)
	{
		if(!head_isset && *(src+i) == '{')
		{
			head_isset = 1;
			head_pos = i;
		}
		else if(head_isset && i > head_pos+2 && *(src+i) == '}')
		{
			char str_field[64] = {0};
			memcpy(str_field, src+tail_pos, head_pos-tail_pos);
			strcat(dst, str_field);
			
			confval_list *t_confval = pval;
			while(t_confval != NULL)
			{
				if(t_confval->head_pos == head_pos)
				{
					strcat(dst, get_val_from_name(t_confval->val));
				}
				t_confval = t_confval->next;
			}

			tail_pos = i + 1;
			head_isset = 0;
		}
	}

	if(tail_pos < vallen)
	{
		char str_field[64] = {0};
		memcpy(str_field, src+tail_pos, vallen-tail_pos);
		strcat(dst, str_field);
	}

	get_confval_free(pval);
}

confval_list *get_confval_alloc_from_str(char *str)
{
	int i;
	int vallen = strlen(str);
	int head_isset = 0;
	int head_pos;
	confval_list *pval = NULL;
	
	for(i=0; i<vallen; i++)
	{
		if(!head_isset && *(str+i) == '{')
		{
			head_isset = 1;
			head_pos = i;
		}
		else if(head_isset && i > head_pos+2 && *(str+i) == '}')
		{
			int j;
			int valhead_isset = 0;
			int valhead_pos;

			for(j=head_pos+1; j<=i; j++)
			{
				if(!valhead_isset && *(str+j) == '$')
				{
					valhead_pos = j;
					valhead_isset = 1;
				}
				else if(valhead_isset
						&& (j == i
							|| *(str+j) == ' '
							|| *(str+j) == ','
							|| *(str+j) == ';'
							|| *(str+j) == '$'))
				{
					if(j-valhead_pos > 1)
					{
						int valname_len = j - valhead_pos - 1;
						char *valname = calloc(1, valname_len+1);
						memcpy(valname, str+valhead_pos+1, valname_len);
						confval_list *m_confval = calloc(1, sizeof(confval_list));
						m_confval->head_pos = head_pos;
						m_confval->val = valname;
						m_confval->next = NULL;

						if(pval == NULL)
						{
							pval = m_confval;
						}
						else
						{
							confval_list *t_confval = pval;
							while(t_confval->next != NULL)
							{
								t_confval = t_confval->next;
							}
							
							t_confval->next = m_confval;
						}
					}

					if(*(str+j) == '$')
					{
						valhead_pos = j;
					}
					else
					{
						valhead_isset = 0;
					}
				}
			}

			head_isset = 0;
		}
	}

	return pval;
}


void get_confval_free(confval_list *pval)
{
	confval_list *t_confval = pval;
	while(t_confval != NULL)
	{
		confval_list *pre_confval = t_confval;
		t_confval = t_confval->next;
		
		free(pre_confval->val);
		free(pre_confval);
	}
}

char *get_val_from_name(char *name)
{
	if(!strcmp(name, "server_ip"))
	{
#ifdef LOAD_BALANCE_SUPPORT
		return get_server_ip();
#else
		return SERVER_IP;
#endif
	}
#ifdef LOAD_BALANCE_SUPPORT
	else if(!strcmp(name+strlen(name)-3, "_ip"))
	{
		return get_server_ip_from_name(name);
	}
#endif
	else if(!strcmp(name, "default_tcp_port"))
	{
		bzero(port_buf, sizeof(port_buf));
		sprintf(port_buf, "%d", TRANS_TCP_PORT);
		return port_buf;
	}
	else if(!strcmp(name, "default_udp_port"))
	{
		bzero(port_buf, sizeof(port_buf));
		sprintf(port_buf, "%d", TRANS_UDP_PORT);
		return port_buf;
	}

	return "";
}

#ifdef REMOTE_UPDATE_APK
void reapk_version_code(char *up_flags, char *ipaddr, cidentify_no_t cidentify_no)
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
			
			if(f_len > 12 && !strncmp(ep->d_name+f_len-4, ".apk", 4))
			{
				if(!strncmp(ep->d_name, "SHomely_", 8))
				{
					char codestr[16] = {0};
					memcpy(codestr, ep->d_name+8, f_len-12);
					int tcode = atoi(codestr);
					version_code = version_code>tcode?version_code:tcode;
				}
				else if(up_flags != NULL && !strncmp(up_flags, "LS", 2)
					&& !strncmp(ep->d_name, "loongsmart_", 11))
				{
					char codestr[16] = {0};
					memcpy(codestr, ep->d_name+11, f_len-15);
					int tcode = atoi(codestr);
					version_code = version_code>tcode?version_code:tcode;
				}
			}
		}
	}

	char data[4];
	incode_xtoc16(data, version_code);
	
	ub.data = data;
	ub.data_len = 4;

	enable_datalog_atime();
	send_frame_udp_request(ipaddr, TRHEAD_UB, &ub);
}
#endif

char *get_current_time()
{
	time_t t;
	time(&t);
	bzero(current_time, sizeof(current_time));
	struct tm *tp= localtime(&t);
	strftime(current_time, 100, "%Y-%m-%d %H:%M:%S", tp); 

	return current_time;
}

char *get_system_time()
{
	struct timeval time;
	gettimeofday(&time, NULL);
	bzero(current_time, sizeof(current_time));
	sprintf(current_time, "%u", time.tv_sec);

	return current_time;
}

char *get_md5(char *text, int result_len)
{
	if(text == NULL)
	{
		return "";
	}

	MD5_CTX ctx;
	uint8 md[16];

	MD5Init(&ctx);
	MD5Update(&ctx,(unsigned char *)text, strlen(text));
	MD5Final(md, &ctx);

	result_len = result_len < 16 ? result_len : 16;
	bzero(curcode, sizeof(curcode));
	incode_xtocs(curcode, md, result_len);

	return curcode;
}

#endif


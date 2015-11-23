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
#include <protocol/common/session.h>
#include <protocol/common/mevent.h>
#include <protocol/protocol.h>

static uint8 _broadcast_no[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
#ifdef COMM_CLIENT
static char serial_port[16] = TRANS_SERIAL_DEV;
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
#ifdef SERIAL_SUPPORT
	TRANS_SERIAL_DEV,
#endif
#ifdef TRANS_TCP_SERVER
	TRANS_TCP_PORT,
#endif
#if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG)
	TRANS_UDP_PORT,
#endif
#ifdef REMOTE_UPDATE_APK
	TRANS_UPDATE_DIR,
#endif
#ifdef DB_API_SUPPORT
	TRANS_DB_NAME,
	TRANS_DB_USER,
	TRANS_DB_PASS
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
    #if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG)
	char *optstrs = "s:t:u:h";
	#else
	char *optstrs = "s:t:h";
    #endif
  #elif defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG)
	char *optstrs = "s:u:h";
  #else
  	char *optstrs = "s:h";
  #endif
#elif defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
  #if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG)
  	char *optstrs = "t:u:h";
  #else
  	char *optstrs = "t:h";
  #endif
#elif defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG)
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
    #if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG)
			DE_PRINTF(0, "Usage: %s [-s<Serial Device>] [-t<TCP Port>] [-u<UDP Port>]\n", argv[0]);
			DE_PRINTF(0, "Default: \n\t-s %s\n\t-t %d\n\t-u %d\n",
							TRANS_SERIAL_DEV,
							TRANS_TCP_PORT,
							TRANS_UDP_PORT);
	#else
			DE_PRINTF(0, "Usage: %s [-s<Serial Device>] [-t<TCP Port>]\n", argv[0]);
			DE_PRINTF(0, "Default:\n\t-s %s\n\t-t %d\n", TRANS_SERIAL_DEV, TRANS_TCP_PORT);
    #endif
  #elif defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG)
			DE_PRINTF(0, "Usage: %s [-s<Serial Device>] [-u<UDP Port>]\n", argv[0]);
  			DE_PRINTF(0, "Default:\n\t-s %s\n\t-u %d\n", TRANS_SERIAL_DEV, TRANS_UDP_PORT);
  #else
  			DE_PRINTF(0, "Usage: %s [-s<Serial Device>]\n", argv[0]);
  			DE_PRINTF(0, "Default:\n\t-s %s\n", TRANS_SERIAL_DEV);
  #endif
#elif defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
  #if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG)
  			DE_PRINTF(0, "Usage: %s [-t<TCP Port>] [-u<UDP Port>]\n", argv[0]);
  			DE_PRINTF(0, "Default:\n\t-t %d\n\t-u %d\n", TRANS_TCP_PORT, TRANS_UDP_PORT);
  #else
  			DE_PRINTF(0, "Usage: %s [-t<TCP Port>]\n", argv[0]);
  			DE_PRINTF(0, "Default:\n\t-t %d\n", TRANS_TCP_PORT);
  #endif
#elif defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG)
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
			sprintf(t_conf.serial_port, "%s", optarg);
			t_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_SERIAL;
			break;
#endif
#if defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
		case 't':
			t_conf.tcp_port = atoi(optarg);
			t_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_TCP;
			break;
#endif
#if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG)
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
		set_serial_port(t_conf.serial_port);
	}
	else
	{
  #ifdef READ_CONF_FILE
		set_serial_port(g_conf.serial_port);
  #else
		set_serial_port(TRANS_SERIAL_DEV);
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
	
    #if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG)

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
	
  #elif defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG)
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

  #if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG)
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
  
#elif defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG)
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
#else
#warning "No Comm protocol be selected, please set uart, tcp or udp."
#endif

#ifdef COMM_CLIENT
	DE_PRINTF(1, "Gateway Start!\n");
#else
	DE_PRINTF(1, "Server Start!\n");
#endif

#ifdef SERIAL_SUPPORT
	DE_PRINTF(1, "Serial port device:\"%s\"\n", get_serial_port());
#endif

#if defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
	DE_PRINTF(1, "TCP transmit port:%d\n", get_tcp_port());
#endif

#if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG)
	DE_PRINTF(1, "UDP transmit port:%d\n", get_udp_port());
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
	set_session_status(SESS_INIT);
	
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

#ifdef COMM_SERVER
	gw_list_t *p_gw_list = get_gateway_list();
	p_gw_list->p_gw = NULL;
	p_gw_list->max_num = 0;

	if(pthread_mutex_init(&(get_gateway_list()->lock), NULL) != 0)
    {
        DE_PRINTF(1, "%s()%d :  pthread_mutext_init failed, errno:%d, error:%s\n",
            __FUNCTION__, __LINE__, errno, strerror(errno));
        return -1;
    }
#endif

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
#ifdef SERIAL_SUPPORT
	if(!strcmp(cmd, GLOBAL_CONF_SERIAL_PORT))
	{
		strcpy(g_conf.serial_port, val);
		g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_SERIAL;
	}
#endif

#if defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
	if(!strcmp(cmd, GLOBAL_CONF_TCP_PORT))
	{
		g_conf.tcp_port = atoi(val);
		g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_TCP;
	}
#endif

#if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG)
	if(!strcmp(cmd, GLOBAL_CONF_UDP_PORT))
	{
		g_conf.udp_port = atoi(val);
		g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_UDP;
	}
#endif

#ifdef REMOTE_UPDATE_APK
	if(!strcmp(cmd, GLOBAL_CONF_UPAPK_DIR))
	{
		strcpy(g_conf.upapk_dir, val);
		g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_UPAPK;
	}
#endif

#ifdef DB_API_SUPPORT
	if(!strcmp(cmd, GLOBAL_CONF_DATABASE))
	{
		strcpy(g_conf.db_name, val);
		g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_DB;
	}
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
}

int get_conf_setval()
{
	int i;
	uint32 issetflags[] = {
#ifdef SERIAL_SUPPORT
					GLOBAL_CONF_ISSETVAL_SERIAL,
#endif
#if defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
					GLOBAL_CONF_ISSETVAL_TCP,
#endif
#if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG)
					GLOBAL_CONF_ISSETVAL_UDP,
#endif
#ifdef REMOTE_UPDATE_APK
					GLOBAL_CONF_ISSETVAL_UPAPK,
#endif
#ifdef DB_API_SUPPORT
					GLOBAL_CONF_ISSETVAL_DB,
					GLOBAL_CONF_ISSETVAL_DBUSER,
					GLOBAL_CONF_ISSETVAL_DBPASS,
#endif
					};

	char *issetvals[] = {
#ifdef SERIAL_SUPPORT
					GLOBAL_CONF_SERIAL_PORT,
#endif
#if defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
					GLOBAL_CONF_TCP_PORT,
#endif
#if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG)
					GLOBAL_CONF_UDP_PORT,
#endif
#ifdef REMOTE_UPDATE_APK
					GLOBAL_CONF_UPAPK_DIR,
#endif
#ifdef DB_API_SUPPORT
					GLOBAL_CONF_DATABASE,
					GLOBAL_CONF_DBUSER,
					GLOBAL_CONF_DBPASS,
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
#endif


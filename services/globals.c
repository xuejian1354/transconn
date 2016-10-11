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
#include <module/netapi.h>
#include <protocol/common/fieldlysis.h>
#include <protocol/common/mevent.h>
#include <protocol/protocol.h>

#ifdef __cplusplus
extern "C" {
#endif

char cmdline[CMDLINE_SIZE];

static char current_time[64];
static char curcode[64];
static uint8 _broadcast_no[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static char port_buf[16];


static char serial_dev[16] = TRANS_SERIAL_DEV;
static char server_ip[IP_ADDR_MAX_SIZE] = SERVER_IP;
static int tcp_port = TRANS_TCP_PORT;
static int udp_port = TRANS_UDP_REMOTE_PORT;

#ifdef DE_TRANS_UDP_STREAM_LOG
static char de_buf[0x4000];
#endif

static global_conf_t g_conf = 
{
	0,
	TRANS_SERIAL_DEV,
	SERVER_IP,
	UPCHECK_TIME,
	QUERYDEV_TIME,
	QUERYDEV_NUM,
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

char *get_serial_dev()
{
	return serial_dev;
}

void set_serial_dev(char *name)
{
	memset(serial_dev, 0, sizeof(serial_dev));
	strcpy(serial_dev, name);
}

#if defined(COMM_TARGET)
char *get_server_ip(void)
{
	return server_ip;
}

int get_tcp_port()
{
	return tcp_port;
}

int get_udp_port()
{
	return udp_port;
}

void set_server_ip(char *ip)
{
	memset(server_ip, 0, sizeof(server_ip));
	strcpy(server_ip, ip);
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

int start_params(int argc, char **argv)
{

	int ch;
	opterr = 0;  
	global_conf_t t_conf = {0};

	const char *optstrs = "s:i:h";
	
    while((ch = getopt(argc, argv, optstrs)) != -1)
    {
		switch(ch)
		{
		case 'h':
			DE_PRINTF(0, "Usage: %s [-s<Serial Device>] [-i<Server IP>]\n", argv[0]);
			DE_PRINTF(0, "Default: \n\t-s %s\n\t-i %d\n",
							TRANS_SERIAL_DEV,
							SERVER_IP);
			return 1;

		case '?':
			DE_PRINTF(0, "Unrecognize arguments.\n");
			DE_PRINTF(0, "\'%s -h\' get more help infomations.\n", argv[0]);
			return 1;

		case 's':
			sprintf(t_conf.serial_dev, "%s", optarg);
			t_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_SERIAL;
			break;

		case 'i':
			sprintf(t_conf.server_ip, "%s", optarg);
			t_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_IP;
			break;
		}
	}

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
	
  	if(t_conf.isset_flag & GLOBAL_CONF_ISSETVAL_IP)
  	{
		set_server_ip(t_conf.server_ip);
  	}
	else
	{
	#ifdef READ_CONF_FILE
		set_server_ip(g_conf.server_ip);
    #else
		set_server_ip(SERVER_IP);
    #endif
	}

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
	gw_info_t *p_gw = get_gateway_info();

	memset(p_gw->gw_no, 0, sizeof(p_gw->gw_no));
	memcpy(p_gw->gw_no, getMac("eth0.2")+3, sizeof(zidentify_no_t));

	p_gw->type = FRAPP_CONNECTOR;
	p_gw->p_dev = NULL;
	p_gw->next = NULL;

	if(pthread_mutex_init(&(get_gateway_info()->lock), NULL) != 0)
    {
        DE_PRINTF(1, "%s()%d :  pthread_mutext_init failed, errno:%d, error:%s\n",
            __FUNCTION__, __LINE__, errno, strerror(errno));
        return -1;
    }

	return 0;
}

void event_init()
{
#ifdef TIMER_SUPPORT
	gateway_init();
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
	g_conf.upcheck_time = UPCHECK_TIME;
	g_conf.querydev_time = QUERYDEV_TIME;
	g_conf.querydev_num = QUERYDEV_NUM;

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
	if(!strcmp(cmd, GLOBAL_CONF_SERIAL_PORT))
	{
		strcpy(g_conf.serial_dev, val);
		g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_SERIAL;
	}

	if(!strcmp(cmd, GLOBAL_CONF_SERVER_IP))
	{
		confval_list *pval = get_confval_alloc_from_str(val);
		if(pval != NULL)
		{
			strcpy(g_conf.server_ip, get_val_from_name(pval->val));
			g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_IP;
			get_confval_free(pval);
		}
		else
		{
			strcpy(g_conf.server_ip, val);
			g_conf.isset_flag |= GLOBAL_CONF_ISSETVAL_IP;
		}
	}

	if(!strcmp(cmd, GLOBAL_CONF_UPCHECK_TIME))
	{
		g_conf.upcheck_time = atol(val);
	}

	if(!strcmp(cmd, GLOBAL_CONF_QUERYDEV_TIME))
	{
		g_conf.querydev_time = atol(val);
	}

	if(!strcmp(cmd, GLOBAL_CONF_QUERYDEV_NUM))
	{
		g_conf.querydev_num = atoi(val);
	}
}

int get_conf_setval()
{
	int i;
	uint32 issetflags[] = {
					GLOBAL_CONF_ISSETVAL_SERIAL,
					GLOBAL_CONF_ISSETVAL_IP,
					};

	const char *issetvals[] = {
					GLOBAL_CONF_SERIAL_PORT,
					GLOBAL_CONF_SERVER_IP,
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
						char *valname = (char *)calloc(1, valname_len+1);
						memcpy(valname, str+valhead_pos+1, valname_len);
						confval_list *m_confval = (confval_list *)calloc(1, sizeof(confval_list));
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
	if(!strcmp(name, "default_ip"))
	{
		return SERVER_IP;
	}

	return (char *)"";
}

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
		return (char *)"";
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

#ifdef __cplusplus
}
#endif


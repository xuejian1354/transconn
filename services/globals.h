/* globals.h
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
#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include <mconfig.h>
#include <debug/dconfig.h>
#include <debug/dlog.h>
#include <signal.h>
#include <tpool.h>
#include <strings_t.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <mincode.h>
#include "time.h"

#ifdef __cplusplus
extern "C" {
#endif

//default program name
#ifndef TARGET_NAME
#define TARGET_NAME "cullive(default)"
#endif

#define SERVER_IP	"classyun.com"

#define SERVER_GW_LIST_MAX_NUM	1024
#define SERVER_CLI_LIST_MAX_NUM		128
#define GATEWAY_CLI_LIST_MAX_NUM	24

#define ZDEVICE_MAX_NUM		256

//max connection size of tcp
#define TRANS_TCP_CONN_MAX_SIZE		1024

//max udp session size of udp
#define TRANS_UDP_SESS_MAX_SIZE		1024

#if defined(TRANS_TCP_SERVER) || defined(UART_COMMBY_SOCKET)
#define TRANS_TCP_CONN_LIST
#endif

//serial dev
#define TRANS_SERIAL_DEV	"/dev/ttyS1"
//replace uart server socket port
#define UART_REPORT		12388

//tcp protocol using port
#define TRANS_TCP_PORT	11665

//udp protocol using port
#define TRANS_UDP_PORT	11678

//websocket protocol using port
#define TRANS_WS_PORT	8020

#ifdef COMM_TARGET
//udp protocol using port
#define TRANS_UDP_SELF_PORT	11678
#define TRANS_UDP_REMOTE_PORT	11678
#endif

#if defined(DE_TRANS_UDP_STREAM_LOG)
#define DEU_UDP_CMD			"deudp"
#define DEU_TCP_CMD			"detcp"
#define DEU_POST_CMD		"depost"
#define DEU_UART_CMD		"deuart"
#define DE_UDP_PORT			13688
#define DE_UDP_CTRL_PORT	13689
#endif

#define TRANS_UDP_TIMEOUT	30000
#define TRANS_TCP_TIMEOUT	10000
#define TRANS_HTTP_TIMEOUT	1000
#define TRANS_WS_TIMEOUT	30000

#define GLOBAL_CONF_SERIAL_PORT		"serial_dev"
#define GLOBAL_CONF_SERVER_IP			"server_ip"

#define GLOBAL_CONF_ISSETVAL_SERIAL			0x00000001
#define GLOBAL_CONF_ISSETVAL_IP				0x00000002

//transport layer listening connection max number
#define TRANS_THREAD_MAX_NUM		6

//single frame max size
#define MAXSIZE	0x4000

#define IP_ADDR_MAX_SIZE	24

#define CMDLINE_SIZE	MAXSIZE
#define GET_CMD_LINE()	cmdline

#define SET_CMD_LINE(format, args...)  	\
st(  									\
	bzero(cmdline, sizeof(cmdline));  	\
	sprintf(cmdline, format, ##args);  	\
)

typedef byte zidentify_no_t[3];

typedef struct
{
	uint32 isset_flag;

	char serial_dev[16];
	char server_ip[IP_ADDR_MAX_SIZE];

}global_conf_t;

typedef struct ConfVal
{
	int head_pos;
	char *val;
	struct ConfVal *next;
}confval_list;

#ifdef DE_TRANS_UDP_STREAM_LOG
char *get_de_buf();
#endif

char *get_serial_dev();
void set_serial_dev(char *name);

char *get_server_ip(void);
int get_tcp_port();
int get_udp_port();

void set_server_ip(char *ip);
void set_tcp_port(int port);
void set_udp_port(int port);

uint8 *get_broadcast_no();

int start_params(int argc, char **argv);
char *get_time_head();
#ifdef DAEMON_PROCESS_CREATE
int daemon_init();
int get_daemon_cmdline();
#endif
int mach_init();
void event_init();

global_conf_t *get_global_conf();
#ifdef READ_CONF_FILE
int conf_read_from_file();
void translate_confval_to_str(char *dst, char *src);
confval_list *get_confval_alloc_from_str(char *str);
void get_confval_free(confval_list *pval);
char *get_val_from_name(char *name);
#endif
char *get_current_time();
char *get_system_time();
char *get_md5(char *text, int result_len);

#ifdef __cplusplus
}
#endif

#endif	//__GLOBALS_H__

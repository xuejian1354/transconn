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
#include "time.h"

//default program name
#ifndef TARGET_NAME
#define TARGET_NAME "transconn(default)"
#endif

#define SERVER_IP	"loongsmart.com"

#define SERVER_GW_LIST_MAX_NUM	1024
#define SERVER_CLI_LIST_MAX_NUM		128
#define GATEWAY_CLI_LIST_MAX_NUM	24

#define ZDEVICE_MAX_NUM		256

//max connection size of tcp
#define TRANS_TCP_CONN_MAX_SIZE		1024

//max udp session size of udp
#define TRANS_UDP_SESS_MAX_SIZE		1024

#ifdef TRANS_TCP_SERVER
#define TRANS_TCP_CONN_LIST
#endif

//serial dev
#define TRANS_SERIAL_DEV	"/dev/ttyS1"

//tcp protocol using port
#define TRANS_TCP_PORT	11565

//udp protocol using port
#define TRANS_UDP_PORT	11578

//path for android apk path
#define TRANS_UPDATE_DIR	"/mnt/android_apk"

//database configuration
#define TRANS_DB_NAME	"loongsmart"
#define TRANS_DB_USER	"root"
#define TRANS_DB_PASS	"root"

#ifdef COMM_CLIENT
//udp protocol using port
#define TRANS_UDP_SELF_PORT	11578
#define TRANS_UDP_REMOTE_PORT	11578
#endif

#ifdef DE_TRANS_UDP_STREAM_LOG
#define DEU_UDP_CMD		"deudp"
#define DEU_TCP_CMD		"detcp"
#define DEU_UART_CMD	"deuart"
#define DE_UDP_PORT		13688
#endif

#define TRANS_UDP_TIMEOUT	30
#define TRANS_TCP_TIMEOUT	10
#define TRANS_HTTP_TIMEOUT	1

#define GLOBAL_CONF_COMM_PROTOCOL	"comm_protocol"
#define GLOBAL_CONF_SERIAL_PORT		"serial_dev"
#define GLOBAL_CONF_MAIN_IP			"main_ip"
#define GLOBAL_CONF_TCP_PORT		"tcp_port"
#define GLOBAL_CONF_UDP_PORT		"udp_port"
#define GLOBAL_CONF_HTTP_URL		"http_url"

#define GLOBAL_CONF_TCP_TIMEOUT		"tcp_timeout"
#define GLOBAL_CONF_UDP_TIMEOUT		"udp_timeout"
#define GLOBAL_CONF_HTTP_TIMEOUT	"http_timeout"

#define GLOBAL_CONF_UPAPK_DIR		"apk_update_dir"

#define GLOBAL_CONF_DATABASE		"db_database"
#define GLOBAL_CONF_DBUSER			"db_username"
#define GLOBAL_CONF_DBPASS			"db_password"

#define GLOBAL_CONF_ISSETVAL_PROTOCOL		0x00000001
#define GLOBAL_CONF_ISSETVAL_SERIAL			0x00000002
#define GLOBAL_CONF_ISSETVAL_IP				0x00000004
#define GLOBAL_CONF_ISSETVAL_TCP			0x00000008
#define GLOBAL_CONF_ISSETVAL_TCP_TIMEOUT	0x00000010
#define GLOBAL_CONF_ISSETVAL_UDP			0x00000020
#define GLOBAL_CONF_ISSETVAL_UDP_TIMEOUT	0x00000040
#define GLOBAL_CONF_ISSETVAL_HTTPURL		0x00000080
#define GLOBAL_CONF_ISSETVAL_HTTP_TIMEOUT	0x00000100
#define GLOBAL_CONF_ISSETVAL_UPAPK			0x00000200
#define GLOBAL_CONF_ISSETVAL_DB				0x00000400
#define GLOBAL_CONF_ISSETVAL_DBUSER			0x00000800
#define GLOBAL_CONF_ISSETVAL_DBPASS			0x00001000

#define GLOBAL_TRANSTOCOL_SIZE		4


/*
Old version not ed_type on gateway frame
This macro just support that
*/
//transport layer listening connection max number
#define TRANS_SERVER_THREAD_MAX_NUM		12
#define TRANS_CLIENT_THREAD_MAX_NUM		6

//single frame max size
#define MAXSIZE	0x4000

#define IP_ADDR_MAX_SIZE	24

#ifdef COMM_CLIENT
#define GET_UDP_SERVICE_IPADDR(ipaddr)								\
st(															\
	sprintf(ipaddr, "%s:%d", get_server_ip(), get_udp_port());	\
)
#endif

#ifdef COMM_SERVER
#define GET_UDP_SERVICE_IPADDR(ipaddr)								\
st(															\
	sprintf(ipaddr, "%s:%d", "0.0.0.0", get_udp_port());	\
)
#endif

#define CMDLINE_SIZE	0x4000
#define GET_CMD_LINE()	cmdline

#define SET_CMD_LINE(format, args...)  	\
st(  									\
	bzero(cmdline, sizeof(cmdline));  	\
	sprintf(cmdline, format, ##args);  	\
)

typedef byte zidentify_no_t[8];
typedef byte cidentify_no_t[8];

typedef struct
{
	uint32 isset_flag;
	uint32 protocols[GLOBAL_TRANSTOCOL_SIZE];
	int tocol_len;

#ifdef SERIAL_SUPPORT
	char serial_dev[16];
#endif

#ifdef COMM_CLIENT
	char main_ip[IP_ADDR_MAX_SIZE];
#endif

#if defined(TRANS_TCP_SERVER) || defined(TRANS_TCP_CLIENT)
	int tcp_port;
#ifdef COMM_CLIENT
	int tcp_timeout;
#endif
#endif

#if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG)
	int udp_port;
#ifdef COMM_CLIENT
	int udp_timeout;
#endif
#endif

#ifdef TRANS_HTTP_REQUEST
	char http_url[1024];
#ifdef COMM_CLIENT
	int http_timeout;
#endif
#endif

#ifdef REMOTE_UPDATE_APK
	char upapk_dir[64];
#endif

#ifdef DB_API_SUPPORT
#if defined(DB_API_WITH_MYSQL) || defined(DB_API_WITH_SQLITE)
	char db_name[32];
#ifdef DB_API_WITH_MYSQL
	char db_user[32];
	char db_password[64];
#endif
#endif
#endif
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

#ifdef COMM_CLIENT
char *get_serial_dev();
void set_serial_dev(char *name);
#endif

#if defined(COMM_CLIENT) || defined(COMM_SERVER)
int get_tcp_port();
int get_udp_port();
void set_tcp_port(int port);
void set_udp_port(int port);
#endif

uint8 *get_broadcast_no();

#ifdef COMM_SERVER
uint8 *get_common_no();
#endif

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
#ifdef REMOTE_UPDATE_APK
void reapk_version_code(char *up_flags, char *ipaddr, cidentify_no_t cidentify_no);
#endif
#endif
char *get_current_time();
char *get_system_time();
char *get_md5(char *text, int result_len);
#endif	//__GLOBALS_H__

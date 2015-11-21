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

#define GLOBAL_CONF_SERIAL_PORT		"serial_port"
#define GLOBAL_CONF_TCP_PORT		"tcp_port"
#define GLOBAL_CONF_UDP_PORT		"udp_port"

#define GLOBAL_CONF_UPAPK_DIR		"apk_update_dir"

#define GLOBAL_CONF_DATABASE		"db_database"
#define GLOBAL_CONF_DBUSER			"db_username"
#define GLOBAL_CONF_DBPASS			"db_password"

#define GLOBAL_CONF_ISSETVAL_SERIAL		0x00000001
#define GLOBAL_CONF_ISSETVAL_TCP		0x00000002
#define GLOBAL_CONF_ISSETVAL_UDP		0x00000004
#define GLOBAL_CONF_ISSETVAL_UPAPK		0x00000008
#define GLOBAL_CONF_ISSETVAL_DB			0x00000010
#define GLOBAL_CONF_ISSETVAL_DBUSER		0x00000020
#define GLOBAL_CONF_ISSETVAL_DBPASS		0x00000040


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

typedef byte zidentify_no_t[8];
typedef byte cidentify_no_t[8];

#ifdef READ_CONF_FILE
typedef struct
{
	uint32 isset_flag;
#ifdef SERIAL_SUPPORT
	char serial_port[16];
#endif

#ifdef TRANS_TCP_SERVER
	int tcp_port;
#endif

#if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG)
	int udp_port;
#endif

#ifdef REMOTE_UPDATE_APK
	char upapk_dir[64];
#endif

#ifdef DB_API_SUPPORT
	char db_name[32];
	char db_user[32];
	char db_password[64];
#endif
}global_conf_t;
#endif

#ifdef DE_TRANS_UDP_STREAM_LOG
char *get_de_buf();
#endif

#ifdef COMM_CLIENT
char *get_serial_port();
void set_serial_port(char *name);
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

#ifdef READ_CONF_FILE
global_conf_t *get_global_conf();
int conf_read_from_file();

#ifdef REMOTE_UPDATE_APK
void reapk_version_code(char *up_flags, char *ipaddr, cidentify_no_t cidentify_no);
#endif
#endif

#endif	//__GLOBALS_H__

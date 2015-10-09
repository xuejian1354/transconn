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

//default program name
#ifndef TARGET_NAME
#define TARGET_NAME "transconn(default)"
#endif

#define SERVER_GW_LIST_MAX_NUM	1024
#define SERVER_CLI_LIST_MAX_NUM		128
#define GATEWAY_CLI_LIST_MAX_NUM	24

#define ZDEVICE_MAX_NUM		128

//max connection size of tcp
#define TRANS_TCP_CONN_MAX_SIZE		1024

//max udp session size of udp
#define TRANS_UDP_SESS_MAX_SIZE		1024

#ifdef TRANS_TCP_SERVER
#define TRANS_TCP_CONN_LIST
#endif

#ifdef TRANS_UDP_SERVICE
//#define TRANS_UDP_SESS_QUEUE
#endif

//tcp protocol using port
#define TRANS_TCP_PORT	11565

//udp protocol using port
#define TRANS_UDP_PORT	11578

//client test use udp port
#define TRANS_CLIENT_TEST_UDP_PORT	12577

#ifdef COMM_CLIENT
//udp protocol using port
#define TRANS_UDP_SELF_PORT	11578
#define TRANS_UDP_REMOTE_PORT	11578
#endif

#ifdef DE_TRANS_UDP_STREAM_LOG
#define DEU_CMD_PREFIX	"set deflag="
#define DE_UDP_PORT		12688
#endif

#define GLOBAL_CONF_UPAPK_DIR	"shomely_update_dir"

/*
Old version not ed_type on gateway frame
This macro just support that
*/
//#define LACK_EDTYPE_SUPPORT

//transport layer listening connection max number
#define TRANS_THREAD_MAX_NUM	5

//single frame max size
#define MAXSIZE	1024

#define IP_ADDR_MAX_SIZE	24

#define GET_SERVER_IP(ipaddr)								\
st(															\
	sprintf(ipaddr, "%s:%d", get_server_ip(), get_udp_port());	\
)

typedef byte zidentify_no_t[8];
typedef byte cidentify_no_t[8];

#ifdef READ_CONF_FILE
typedef struct
{
#ifdef REMOTE_UPDATE_APK
	char upapk_dir[64];
#endif
}global_conf_t;
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

#ifdef COMM_CLIENT
uint8 *get_broadcast_no();
#endif

#ifdef COMM_SERVER
uint8 *get_common_no();
#endif

int mach_init();
void event_init();

#ifdef CLIENT_TEST
void set_target_ip(char *ipaddr, int len);
#endif

#ifdef READ_CONF_FILE
void conf_read_from_file();

#ifdef REMOTE_UPDATE_APK
void reapk_version_code(char *ipaddr, cidentify_no_t cidentify_no);
#endif
#endif

#endif	//__GLOBALS_H__

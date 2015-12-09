/*
 * netapi.h
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
#ifndef __NETAPI_H__
#define __NETAPI_H__

#include <services/globals.h>
#include <protocol/common/session.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#ifdef TRANS_HTTP_REQUEST
#include <curl/curl.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <sqlite3.h>
#endif

typedef struct
{
	int fd;
	transtocol_t transtocol;
	struct sockaddr_in addr; 
	char *buf;
	int len;
}frhandler_arg_t;

#ifdef TRANS_HTTP_REQUEST
typedef enum
{
	CURL_GET,
	CURL_POST
}curl_method_t;

typedef size_t (*data_handler)(void *, size_t, size_t, void *);

typedef struct
{
	curl_method_t cm;
	char *url;
	char *req;
	data_handler curl_callback;
}curl_args_t;
#endif

frhandler_arg_t *get_frhandler_arg_alloc(int fd,
			transtocol_t transtocol, struct sockaddr_in *addr, char *buf, int len);
void get_frhandler_arg_free(frhandler_arg_t *arg);

#ifdef COMM_CLIENT
frhandler_arg_t *get_transtocol_frhandler_arg();
#endif

#ifdef TRANS_TCP_SERVER
int get_stcp_fd();
int socket_tcp_server_init(int port);
void socket_tcp_server_accept(int fd);
void socket_tcp_server_recv(int fd);
#endif

#ifdef TRANS_TCP_CLIENT
int get_ctcp_fd();
int socket_tcp_client_connect(int port);
void socket_tcp_client_recv();
void socket_tcp_client_send(char *data, int len);
void socket_tcp_client_close();
#endif

#if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG) || defined(DE_TRANS_UDP_CONTROL)
int get_udp_fd();
int socket_udp_service_init(int port);
void socket_udp_sendto_with_ipaddr(char *ipaddr, char *data, int len);
void socket_udp_sendto(struct sockaddr_in *addr, char *data, int len);
void socket_udp_recvfrom();
#endif

void set_deuart_flag(uint8 flag);
int get_deuart_flag();
#ifdef DE_TRANS_UDP_STREAM_LOG
void delog_udp_sendto(char *data, int len);
#endif

#ifdef TRANS_HTTP_REQUEST
size_t curl_data(void *buffer, size_t size, size_t nmemb, void *userp);
void curl_http_request(curl_method_t cm, 
		char *url, char *req, data_handler reback);
#endif

void enable_datalog_atime();

#endif  // __NETAPI_H__
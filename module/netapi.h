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
#include <sys/socket.h>
#include <arpa/inet.h>
#ifdef TRANS_HTTP_REQUEST
#include <curl/curl.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#endif

typedef enum
{
	DE_UDP_SEND,
	DE_UDP_RECV,
	DE_TCP_ACCEPT,
	DE_TCP_SEND,
	DE_TCP_RECV,
	DE_TCP_RELEASE
}de_print_t;

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

#ifdef TRANS_TCP_SERVER
int get_stcp_fd();
int socket_tcp_server_init(int port);
int socket_tcp_server_accept(int fd);
#endif
#if defined(TRANS_TCP_SERVER) || (defined(COMM_CLIENT) && defined(UART_COMMBY_SOCKET))
int socket_tcp_server_recv(int fd);
#endif

#ifdef TRANS_TCP_CLIENT
int get_ctcp_fd();
int socket_tcp_client_connect(int port);
int socket_tcp_client_recv();
void socket_tcp_client_send(char *data, int len);
void socket_tcp_client_close();
#endif

#if defined(TRANS_UDP_SERVICE) || defined(DE_TRANS_UDP_STREAM_LOG)
int get_udp_fd();
int socket_udp_service_init(int port);
void socket_udp_sendto(char *addr, char *data, int len);
int socket_udp_recvfrom();
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

void trans_data_show(de_print_t deprint, 
		struct sockaddr_in *addr, char *data, int len);

void enable_datalog_atime();

#endif  // __NETAPI_H__

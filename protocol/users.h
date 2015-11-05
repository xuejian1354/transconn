/*
 * users.h
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

#ifndef __USERS_H__
#define __USERS_H__

#include <services/globals.h>
#include <protocol/trframelysis.h>

typedef struct Cli_User
{
	char name[64];
	char email[64];
	char *devices;
	char *iscollects;
	char *locates;
	char *areas;
	char *scenes;
}cli_user_t;

typedef struct Cli_Info
{
	cidentify_no_t cidentify_no;
	tr_trans_type_t trans_type;
	uint8 ipaddr[IP_ADDR_MAX_SIZE];
	uint8 ip_len;
	uint8 serverip_addr[IP_ADDR_MAX_SIZE];
	uint8 serverip_len;
	int check_count;
	int check_conn;
	cli_user_t *user_info;
	struct Cli_Info *next;
}cli_info_t;

typedef struct
{
	cli_info_t *p_cli;
	pthread_mutex_t lock;
	long max_num;
}cli_list_t;

cli_list_t *get_client_list();

int add_client_info(cli_info_t *m_info);
cli_info_t *query_client_info(cidentify_no_t cidentify_no);
int del_client_info(cidentify_no_t cidentify_no);

#endif  //__USERS_H__


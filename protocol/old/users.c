/*
 * users.c
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
#include "users.h"

#ifdef __cplusplus
extern "C" {
#endif

static cli_list_t cli_list;

cli_list_t *get_client_list()
{
	return &cli_list;
}

int add_client_info(cli_info_t *m_info)
{
	cli_info_t *pre_before, *pre_cli =  NULL;
	cli_info_t *t_cli = get_client_list()->p_cli;

	if(m_info == NULL)
	{
		return -1;
	}
	else
	{
		m_info->next = NULL;
	}

	while(t_cli != NULL)
	{
		if(memcmp(t_cli->cidentify_no, m_info->cidentify_no, sizeof(cidentify_no_t)))
		{
			pre_before = pre_cli;
			pre_cli = t_cli;
			t_cli = t_cli->next;
		}
		else
		{
			if(m_info->ip_len != 0
				&& t_cli->ipaddr != m_info->ipaddr)
			{
				memset(t_cli->ipaddr, 0, sizeof(t_cli->ipaddr));
				memcpy(t_cli->ipaddr, m_info->ipaddr, m_info->ip_len);
				t_cli->ip_len = m_info->ip_len;
			}
			
			if(m_info->serverip_len != 0
				&& t_cli->serverip_addr != m_info->serverip_addr)
			{
				memset(t_cli->serverip_addr, 0, sizeof(t_cli->serverip_addr));
				memcpy(t_cli->serverip_addr, m_info->serverip_addr, m_info->serverip_len);
				t_cli->serverip_len = m_info->serverip_len;
			}

			if(strlen(m_info->email) > 0)
			{
				bzero(t_cli->email, sizeof(t_cli->email));
				memcpy(t_cli->email, m_info->email, strlen(m_info->email));
			}
			
			t_cli->check_count = m_info->check_count;
			t_cli->check_conn = m_info->check_conn;

			if(pre_cli != NULL)
			{
				pthread_mutex_lock(&get_client_list()->lock);
				pre_cli->next = t_cli->next;
				t_cli->next = get_client_list()->p_cli;
				get_client_list()->p_cli = t_cli;
				pthread_mutex_unlock(&get_client_list()->lock);
			}
			
			return 1;
		}
	}

	pthread_mutex_lock(&get_client_list()->lock);
	m_info->next = get_client_list()->p_cli;
	get_client_list()->p_cli = m_info;

	if(get_client_list()->max_num >= GATEWAY_CLI_LIST_MAX_NUM)
	{
		if(pre_before != NULL)
			pre_before->next = NULL;
		
		free(pre_cli);
	}
	else
	{
		get_client_list()->max_num++;
	}
	pthread_mutex_unlock(&get_client_list()->lock);

	return 0;
}

cli_info_t *query_client_info(cidentify_no_t cidentify_no)
{
	cli_info_t *t_cli = get_client_list()->p_cli;

	while(t_cli != NULL)
	{
		if(memcmp(t_cli->cidentify_no, cidentify_no, sizeof(cidentify_no_t)))
		{
			t_cli = t_cli->next;
		}
		else
		{
			return t_cli;
		}
	}

	return NULL;
}

int del_client_info(cidentify_no_t cidentify_no)
{
	cli_info_t *pre_cli =  NULL;
	cli_info_t *t_cli = get_client_list()->p_cli;

	while(t_cli != NULL)
	{
		if(memcmp(t_cli->cidentify_no, cidentify_no, sizeof(cidentify_no_t)))
		{
			pre_cli = t_cli;
			t_cli = t_cli->next;
		}
		else
		{
			pthread_mutex_lock(&get_client_list()->lock);
			if(pre_cli != NULL)
			{
				pre_cli->next = t_cli->next;
			}
			else
			{
				get_client_list()->p_cli = t_cli->next;
			}
			
			get_client_list()->max_num--;
			pthread_mutex_unlock(&get_client_list()->lock);

			free(t_cli);
			return 0;
		}
	}

	return -1;
}

#ifdef __cplusplus
}
#endif

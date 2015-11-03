/*
 * netlist.c
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

#include "netlist.h"

#ifdef TRANS_TCP_CONN_LIST
static tcp_conn_list_t tcp_conn_list = {
	NULL,
	0,
	TRANS_TCP_CONN_MAX_SIZE,
};
#endif

#ifdef TRANS_UDP_SESS_QUEUE
static udp_sess_queue_t udp_sess_queue = {
	0,
	0,
	0,
	TRANS_UDP_SESS_MAX_SIZE,
	{0},
};
#endif

#ifdef TRANS_TCP_CONN_LIST
int addto_tcpconn_list(tcp_conn_t *list)
{
	tcp_conn_t *t_list;

	if(tcp_conn_list.num >= tcp_conn_list.max_size)
	{
		DE_PRINTF(1, "%s:tcp conn list num is %d, beyond max size\n", 
			__FUNCTION__, tcp_conn_list.num);
		
		return -1;
	}
	
	if(tcp_conn_list.p_head == NULL)
	{
		tcp_conn_list.p_head = list;
		tcp_conn_list.num = 1;
	}
	else
	{
		t_list = tcp_conn_list.p_head;
		while(t_list->next != NULL)
		{
			t_list = t_list->next;
		}
		t_list->next = list;
		tcp_conn_list.num++;
	}

	return 0;
}

tcp_conn_t *queryfrom_tcpconn_list(int fd)
{
	tcp_conn_t *t_list;
	
	if(tcp_conn_list.p_head != NULL)
	{
		for(t_list=tcp_conn_list.p_head; t_list!=NULL; t_list=t_list->next)
		{
			if(t_list->fd == fd)
			{
				return t_list;
			}
		}
	}

	DE_PRINTF(1, "%s:no found connectin in tcp conn list\n", __FUNCTION__);
	return NULL;
}

int delfrom_tcpconn_list(int fd)
{
	tcp_conn_t *t_list, *b_list;
	t_list = tcp_conn_list.p_head;
	b_list = NULL;

	if(tcp_conn_list.num <= 0)
	{
		DE_PRINTF(1, "%s:tcp conn list num is %d, no connection in list\n", 
			__FUNCTION__, tcp_conn_list.num);
		
		return -1;
	}
	
	while(t_list->fd!=fd && t_list->next!=NULL)
	{
		b_list = t_list;
		t_list = t_list->next;
	}

	if(t_list->fd == fd)
	{
		if(b_list == NULL)
			tcp_conn_list.p_head = t_list->next;
		else
			b_list->next = t_list->next;

		free(t_list);
		tcp_conn_list.num--;
		return 0;
	}

	DE_PRINTF(1, "%s:no found connectin in tcp conn list\n", __FUNCTION__);
	return -1;
}
#endif

#ifdef TRANS_UDP_SESS_QUEUE
int addto_udpsess_queue(struct sockaddr_in *addr)
{
	udp_sess_queue.num = udp_sess_queue.max_size;
	if(udp_sess_queue.tail < udp_sess_queue.max_size-1)
	{
		udp_sess_queue.tail++;
	}
	else
	{
		udp_sess_queue.tail = 0;
	}

	memcpy(udp_sess_queue.client_addrs+udp_sess_queue.tail, 
			addr, sizeof(struct sockaddr_in));
	
	if(udp_sess_queue.num >= udp_sess_queue.max_size)
	{
		if(udp_sess_queue.head < udp_sess_queue.max_size-1)
		{
			udp_sess_queue.head++;
		}
		else
		{
			udp_sess_queue.head = 0;
		}
	}
}

int queryfrom_udpsess_queue(struct sockaddr_in *addr)
{

}
#endif
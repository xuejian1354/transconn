/*
 * corecomm.c
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
#include "corecomm.h"
#include <module/netapi.h>

static int maxfd;
static fd_set global_rdfs;

void select_init()
{
	maxfd = 0;
	FD_ZERO(&global_rdfs);
}

void select_set(int fd)
{
	FD_SET(fd, &global_rdfs);
	maxfd = maxfd>fd?maxfd:fd;
}

void select_clr(int fd)
{
	FD_CLR(fd, &global_rdfs);
}

#ifdef COMM_SERVER
int select_listen()
{
	int i, ret;
#ifdef TRANS_UDP_SERVICE
	int udpfd = get_udp_fd();
#endif
#ifdef TRANS_TCP_SERVER
	int tcpfd = get_tcp_fd();
#endif
	
	fd_set current_rdfs = global_rdfs;
	ret = select(maxfd+1, &current_rdfs, NULL, NULL, 0);
	if(ret > 0)
	{
#ifdef TRANS_UDP_SERVICE
		if(FD_ISSET(udpfd, &current_rdfs))
		{
			socket_udp_client_recvfrom(udpfd);
		}
#ifdef TRANS_TCP_SERVER
		else 
#endif
#endif
#ifdef TRANS_TCP_SERVER
		if(FD_ISSET(tcpfd, &current_rdfs))
		{
			socket_tcp_client_connect(tcpfd);
		}
		else
		{
			for(i=0; i<=maxfd; i++)
			{
				if(FD_ISSET(i, &current_rdfs) && i!= tcpfd && i!= udpfd)
				{
					socket_tcp_client_recv(i);
				}
			}
		}
#endif
	}
	else if (ret < 0)
	{
		perror("select");
		return -1;
	}

	return 0;
}
#endif

#ifdef COMM_CLIENT
int select_listen()
{
	int ret;
	
	fd_set current_rdfs = global_rdfs;
	ret = select(maxfd+1, &current_rdfs, NULL, NULL, 0);
	if(ret > 0)
	{
#ifdef TRANS_UDP_SERVICE
		if(FD_ISSET(get_udp_fd(), &current_rdfs))
		{
			socket_udp_recvfrom();
		}
#endif
	}
	else if (ret < 0)
	{
		perror("select");
		return -1;
	}

	return 0;
}
#endif
/*
 * trrequest.c
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
#include "trrequest.h"
#include <protocol/protocol.h>


void pi_handler(struct sockaddr_in *addr, pi_t *pi)
{
	gw_info_t *p_gw;
	dev_info_t *p_dev;
	uint8 *buffer;

	char ipaddr[24] = {0};
	sprintf(ipaddr, "%s:%u", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
	
	switch(pi->trans_type)
	{
	case TRTYPE_UDP:
		switch(pi->fr_type)
		{
		case TRFRAME_CON:
#ifdef COMM_SERVER
			p_gw = query_gateway_info(pi->sn);

			if(p_gw == NULL)
			{
				send_pi_udp_request(ipaddr, TRFRAME_GET, ipaddr, strlen(ipaddr), pi->sn);
			}
			else
			{
				memset(p_gw->ipaddr, 0, sizeof(p_gw->ipaddr));
				memcpy(p_gw->ipaddr, pi->data, pi->data_len);
				
				set_gateway_check(p_gw->gw_no, p_gw->rand);
				send_bi_udp_respond(ipaddr, TRFRAME_REG, ipaddr, strlen(ipaddr), pi->sn);
			}
#endif
			break;

		case TRFRAME_GET:
#ifdef COMM_CLIENT
			p_gw = get_gateway_info();
			p_gw->rand = gen_rand(pi->sn);
			memset(p_gw->ipaddr, 0, sizeof(p_gw->ipaddr));
			memcpy(p_gw->ipaddr, pi->data, pi->data_len);
			p_gw->ip_len = pi->data_len;
			
			buffer = get_gateway_buffer_alloc(p_gw);
			send_bi_udp_respond(ipaddr, 
				TRFRAME_PUT_GW, buffer, GATEWAY_BUFFER_FIX_SIZE+p_gw->ip_len, NULL);
			get_gateway_buffer_free(buffer);

			p_dev = p_gw->p_dev;
			while(p_dev != NULL)
			{
				buffer = get_zdev_buffer_alloc(p_dev);
				send_bi_udp_respond(ipaddr, TRFRAME_PUT_DEV, buffer, ZDEVICE_BUFFER_SIZE, NULL);
				get_zdev_buffer_free(buffer);
			
				p_dev = p_dev->next;
			}
#endif
			break;
		}
		break;

	case TRTYPE_TCP:
		break;
	}
}


void bi_handler(struct sockaddr_in *addr, bi_t *bi)
{
	gw_info_t *p_gw;
	dev_info_t *p_dev;

	char ipaddr[24] = {0};
	sprintf(ipaddr, "%s:%u", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
	
	switch(bi->trans_type)
	{
	case TRTYPE_UDP:
		switch(bi->fr_type)
		{
		case TRFRAME_REG:
#ifdef COMM_CLIENT
			p_gw = get_gateway_info();
			p_gw->rand = gen_rand(bi->sn);
			memset(p_gw->ipaddr, 0, sizeof(p_gw->ipaddr));
			memcpy(p_gw->ipaddr, bi->data, bi->data_len);
			p_gw->ip_len = bi->data_len;
			printf("client conn respond:%x\n", p_gw->rand);
#endif
			break;
			
		case TRFRAME_PUT_GW:
#ifdef COMM_SERVER
			p_gw = get_gateway_frame_alloc(bi->data, bi->data_len);			
			if(p_gw != NULL)
			{
				set_gateway_check(p_gw->gw_no, p_gw->rand);
				if(add_gateway_info(p_gw) != 0)
				{
					get_gateway_frame_free(p_gw);
				}
				else
				{
					printf("server get gw info\n");
				}
			}
#endif
			break;

		case TRFRAME_PUT_DEV:
#ifdef COMM_SERVER
			if((p_gw=query_gateway_info(bi->sn)) != NULL)
			{
				p_dev = get_zdev_frame_alloc(bi->data, bi->data_len);
			
				if(p_dev != NULL && add_zdev_info(p_gw, p_dev) != 0)
				{
					get_zdev_frame_free(p_dev);
				}
				else
				{
					printf("server get dev info\n");
				}
			}
#endif
			break;
		}
		break;

	case TRTYPE_TCP:
		break;
	}
}


void gp_handler(struct sockaddr_in *addr, gp_t *gp)
{
	char ipaddr[24] = {0};
	sprintf(ipaddr, "%s:%u", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
	
#ifdef COMM_SERVER
	gw_info_t *p_gw = get_gateway_list()->p_gw;
	while(p_gw != NULL)
	{
		dev_info_t *p_dev = p_gw->p_dev;
		while(p_dev != NULL)
		{
			if(!memcmp(p_dev->zidentity_no, gp->zidentify_no, sizeof(zidentify_no_t)))
			{
				goto dev_match;
			}
		}
	}

	return;

dev_match:
	send_gp_udp_request(p_gw->ipaddr, ipaddr, strlen(ipaddr));
	return;
#endif

#ifdef COMM_CLIENT
#endif
}

void send_pi_udp_request(char *ipaddr, 
	tr_frame_type_t trfra, char *data, int len, uint8 *sn)
{
	pi_t pi;
	tr_buffer_t *buffer;

	switch(trfra)
	{
	case TRFRAME_CON:
#ifdef COMM_CLIENT
		memcpy(pi.sn, get_gateway_info()->gw_no, sizeof(zidentify_no_t));
		break;
#else
		return;
#endif

	case TRFRAME_REG:
		return;
		
	case TRFRAME_GET:
#ifdef COMM_SERVER
		memcpy(pi.sn, sn, sizeof(zidentify_no_t));
		break;
#else
		return;
#endif
		
	case TRFRAME_PUT_GW:
		return;

	case TRFRAME_PUT_DEV:
		return;

	default:
		return;
	}

	pi.trans_type = TRTYPE_UDP;
	pi.fr_type = trfra;
	pi.data = data;
	pi.data_len = len;
	
	if((buffer = get_trbuffer_alloc(TRHEAD_PI, &pi)) == NULL)
	{
		return;
	}
	
	socket_udp_sendto(ipaddr, buffer->data, buffer->size);
	get_trbuffer_free(buffer);
}


void send_bi_udp_respond(char *ipaddr, 
	tr_frame_type_t trfra, char *data, int len, uint8 *sn)
{
	bi_t bi;
	tr_buffer_t *buffer;

	switch(trfra)
	{
	case TRFRAME_CON:
		return;

	case TRFRAME_REG:
#ifdef COMM_SERVER
		memcpy(bi.sn, sn, sizeof(zidentify_no_t));
		break;
#else
		return;
#endif
		
	case TRFRAME_GET:
		return;
		
	case TRFRAME_PUT_GW:
#ifdef COMM_CLIENT
		memcpy(bi.sn, get_gateway_info()->gw_no, sizeof(zidentify_no_t));
		break;
#else
		return;
#endif

	case TRFRAME_PUT_DEV:
#ifdef COMM_CLIENT
		memcpy(bi.sn, get_gateway_info()->gw_no, sizeof(zidentify_no_t));
		break;
#else
		return;
#endif

	default:
		return;
	}

	bi.trans_type = TRTYPE_UDP;
	bi.fr_type = trfra;
	bi.data = data;
	bi.data_len = len;
	
	if((buffer = get_trbuffer_alloc(TRHEAD_BI, &bi)) == NULL)
	{
		return;
	}
	
	socket_udp_sendto(ipaddr, buffer->data, buffer->size);
	get_trbuffer_free(buffer);
}

void send_gp_udp_request(char *ipaddr, char *data, int len)
{
	
}
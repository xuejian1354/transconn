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
				send_pi_udp_request(ipaddr, TRFRAME_GET, NULL, 0, pi->sn);
			}
			else
			{
				set_gateway_check(p_gw->gw_no, p_gw->rand);
				send_bi_udp_respond(ipaddr, TRFRAME_REG, NULL, 0, pi->sn);
			}
#endif
			break;

		case TRFRAME_GET:
#ifdef COMM_CLIENT
			p_gw = get_gateway_info();
			p_gw->rand = gen_rand(pi->sn);
			
			gw_info_t mgw_info = *p_gw;
			mgw_info.p_dev = NULL;
			mgw_info.next = NULL;
			send_bi_udp_respond(ipaddr, TRFRAME_PUT, (char *)&mgw_info, sizeof(mgw_info), NULL);
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
			printf("client conn respond:%x\n", p_gw->rand);
#endif
			break;
			
		case TRFRAME_PUT:
#ifdef COMM_SERVER
			printf("server get gw info\n");
			p_gw = calloc(1, sizeof(gw_info_t));
			memcpy(p_gw, bi->data, sizeof(p_gw));
			if(add_gateway_info(p_gw) != 0)
			{
				free(p_gw);
			}
#endif
			break;
		}
		break;

	case TRTYPE_TCP:
		break;
	}
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
		
	case TRFRAME_PUT:
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
		
	case TRFRAME_PUT:
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


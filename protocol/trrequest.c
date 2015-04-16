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
				send_bi_udp_respond(ipaddr, 
					TRFRAME_PUT_DEV, buffer, ZDEVICE_BUFFER_SIZE, NULL);
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
#endif
			break;
			
		case TRFRAME_PUT_GW:
#ifdef COMM_SERVER
			p_gw = get_gateway_frame_alloc(bi->data, bi->data_len);
			p_gw->ip_len = strlen(ipaddr);
			memset(p_gw->ipaddr, 0, sizeof(p_gw->ipaddr));
			memcpy(p_gw->ipaddr, ipaddr, p_gw->ip_len);
			if(p_gw != NULL)
			{
				set_gateway_check(p_gw->gw_no, p_gw->rand);
				if(add_gateway_info(p_gw) != 0)
				{
					get_gateway_frame_free(p_gw);
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
			}
#endif
			break;
		}
		break;

	case TRTYPE_TCP:
		break;
	}
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


void gp_handler(struct sockaddr_in *addr, gp_t *gp)
{
	char ipaddr[24] = {0};
	sprintf(ipaddr, "%s:%u", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
	dev_info_t *p_dev = NULL;
	
#ifdef COMM_SERVER
	cli_info_t *m_info = calloc(1, sizeof(cli_info_t));
	memcpy(m_info->cidentify_no, gp->cidentify_no, sizeof(cidentify_no_t));
	memcpy(m_info->ipaddr, ipaddr, strlen(ipaddr));
	m_info->ip_len = strlen(ipaddr);
	m_info->check_count = 0;
	m_info->check_conn = 0;
	m_info->next = NULL;
	
	if(add_client_info(m_info) != 0)
	{
		free(m_info);
	}

	gw_info_t *p_gw = get_gateway_list()->p_gw;
	while(p_gw != NULL)
	{
		p_dev = p_gw->p_dev;
		while(p_dev != NULL)
		{
			if(!memcmp(p_dev->zidentity_no, gp->zidentify_no, sizeof(zidentify_no_t)))
			{
				goto dev_match;
			}

			p_dev = p_dev->next;
		}

		p_gw = p_gw->next;
	}

	send_rp_udp_respond(ipaddr, TRINFO_DATA, gp->zidentify_no, gp->cidentify_no, NULL, 0);
	return;

dev_match:
	send_gp_udp_request(p_gw->ipaddr, TRINFO_IP, 
		gp->zidentify_no, gp->cidentify_no, ipaddr, strlen(ipaddr));

	uo_t m_uo;

	memcpy(m_uo.head, FR_HEAD_UO, 3);
	m_uo.type = get_frnet_type_to_ch(p_dev->znet_type);
	get_frapp_type_to_str(m_uo.ed_type, p_dev->zapp_type);
	incode_xtocs(m_uo.ext_addr, p_dev->zidentity_no, 8);
	incode_xtoc16(m_uo.short_addr, p_dev->znet_addr);
	m_uo.data = NULL;
	m_uo.data_len = 0;
	memcpy(m_uo.tail, FR_TAIL, 4);
	
	fr_buffer_t *frbuffer = get_buffer_alloc(HEAD_UO, &m_uo);

	send_rp_udp_respond(ipaddr, TRINFO_DATA, 
		p_gw->gw_no, gp->cidentify_no, frbuffer->data, frbuffer->size);

	get_buffer_free(frbuffer);
	return;
#endif

#ifdef COMM_CLIENT
	if(gp->data_len > IP_ADDR_MAX_SIZE)
	{
		return;
	}

	cli_info_t *m_info = calloc(1, sizeof(cli_info_t));
	memcpy(m_info->cidentify_no, gp->cidentify_no, sizeof(cidentify_no_t));
	memcpy(m_info->ipaddr, gp->data, gp->data_len);
	m_info->ip_len = gp->data_len;
	m_info->check_count = 3;
	m_info->check_conn = 1;
	m_info->next = NULL;
	
	if(add_client_info(m_info) != 0)
	{
		free(m_info);
	}

	uo_t m_uo;
	if((p_dev=query_zdevice_info_with_sn(gp->zidentify_no)) == NULL)
	{
		return;
	}

	memcpy(m_uo.head, FR_HEAD_UO, 3);
	m_uo.type = get_frnet_type_to_ch(p_dev->znet_type);
	get_frapp_type_to_str(m_uo.ed_type, p_dev->zapp_type);
	incode_xtocs(m_uo.ext_addr, p_dev->zidentity_no, 8);
	incode_xtoc16(m_uo.short_addr, p_dev->znet_addr);
	m_uo.data = NULL;
	m_uo.data_len = 0;
	memcpy(m_uo.tail, FR_TAIL, 4);
	
	fr_buffer_t *frbuffer = get_buffer_alloc(HEAD_UO, &m_uo);
	
	send_rp_udp_respond(m_info->ipaddr, TRINFO_DATA, get_gateway_info()->gw_no, 
		gp->cidentify_no, frbuffer->data, frbuffer->size);
	
	get_buffer_free(frbuffer);

	set_rp_check(m_info);
#endif
}


void send_gp_udp_request(char *ipaddr, tr_info_type_t trinfo,
	zidentify_no_t zidentify_no, cidentify_no_t cidentify_no, char *data, int len)
{
	gp_t gp;
	tr_buffer_t *buffer;

	memcpy(gp.zidentify_no, zidentify_no, sizeof(zidentify_no_t));
	memcpy(gp.cidentify_no, cidentify_no, sizeof(cidentify_no_t));
	gp.tr_info = trinfo;
	gp.data = data;
	gp.data_len = len;

	if((buffer = get_trbuffer_alloc(TRHEAD_GP, &gp)) == NULL)
	{
		return;
	}

	socket_udp_sendto(ipaddr, buffer->data, buffer->size);
	get_trbuffer_free(buffer);
}


void rp_handler(struct sockaddr_in *addr, rp_t *rp)
{
	char ipaddr[24] = {0};
	sprintf(ipaddr, "%s:%u", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
	
#ifdef COMM_CLIENT
	if(memcmp(get_gateway_info()->gw_no, rp->zidentify_no, sizeof(zidentify_no_t))
		|| rp->data_len > IP_ADDR_MAX_SIZE)
	{
		return;
	}

	if(rp->tr_info == TRINFO_CUT)
	{
		set_rp_check(NULL);
	}
#endif

#ifdef COMM_SERVER
	if(rp->data != NULL)
	{
		if(rp->tr_info != TRINFO_IP)
		{
			return;
		}
		
		send_rp_udp_respond(rp->data, TRINFO_UPDATE, 
			rp->zidentify_no, rp->cidentify_no, NULL, 0);
		
		return;
	}
#endif

#ifdef CLIENT_TEST
	set_target_ip(ipaddr, strlen(ipaddr));

	send_rp_udp_respond(ipaddr, TRINFO_CUT, 
		rp->zidentify_no, rp->cidentify_no, NULL, 0);
#endif
}


void send_rp_udp_respond(char *ipaddr, tr_info_type_t trinfo,
	zidentify_no_t zidentify_no, cidentify_no_t cidentify_no, char *data, int len)
{
	rp_t rp;
	tr_buffer_t *buffer;

	memcpy(rp.zidentify_no, zidentify_no, sizeof(zidentify_no_t));
	memcpy(rp.cidentify_no, cidentify_no, sizeof(cidentify_no_t));
	rp.tr_info = trinfo;
	rp.data = data;
	rp.data_len = len;

	if((buffer = get_trbuffer_alloc(TRHEAD_RP, &rp)) == NULL)
	{
		return;
	}

	socket_udp_sendto(ipaddr, buffer->data, buffer->size);
	get_trbuffer_free(buffer);
}


void gd_handler(struct sockaddr_in *addr, gd_t *gd)
{
	char ipaddr[24] = {0};
	sprintf(ipaddr, "%s:%u", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
	
#ifdef COMM_CLIENT
	if(memcmp(get_gateway_info()->gw_no, gd->zidentify_no, sizeof(zidentify_no_t)))
	{
		return;
	}

	send_rd_udp_respond(ipaddr, get_gateway_info()->gw_no, gd->cidentify_no);
#endif


#ifdef CLIENT_TEST
	send_rd_udp_respond(ipaddr, gd->zidentify_no, gd->cidentify_no);
#endif
}


void send_gd_udp_request(char *ipaddr, 
	zidentify_no_t zidentify_no, cidentify_no_t cidentify_no)
{
	gd_t gd;
	tr_buffer_t *buffer;

	memcpy(gd.zidentify_no, zidentify_no, sizeof(zidentify_no_t));
	memcpy(gd.cidentify_no, cidentify_no, sizeof(cidentify_no_t));

	if((buffer = get_trbuffer_alloc(TRHEAD_GD, &gd)) == NULL)
	{
		return;
	}

	socket_udp_sendto(ipaddr, buffer->data, buffer->size);
	get_trbuffer_free(buffer);
}


void rd_handler(struct sockaddr_in *addr, rd_t *rd)
{
#ifdef COMM_CLIENT
	cli_info_t *p_cli = query_client_info(rd->cidentify_no);
	set_cli_check(p_cli);
#endif
}


void send_rd_udp_respond(char *ipaddr, 
	zidentify_no_t zidentify_no, cidentify_no_t cidentify_no)
{
	rd_t rd;
	tr_buffer_t *buffer;

	memcpy(rd.zidentify_no, zidentify_no, sizeof(zidentify_no_t));
	memcpy(rd.cidentify_no, cidentify_no, sizeof(cidentify_no_t));

	if((buffer = get_trbuffer_alloc(TRHEAD_RD, &rd)) == NULL)
	{
		return;
	}

	socket_udp_sendto(ipaddr, buffer->data, buffer->size);
	get_trbuffer_free(buffer);
}


void dc_handler(struct sockaddr_in *addr, dc_t *dc)
{
	char ipaddr[24] = {0};
	sprintf(ipaddr, "%s:%u", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
	
#ifdef COMM_CLIENT
	send_ub_udp_respond(ipaddr, TRINFO_DATA, 
		get_gateway_info()->gw_no, dc->cidentify_no, dc->data, dc->data_len);

	return;
#endif

#ifdef COMM_SERVER
	gw_info_t *p_gw = get_gateway_list()->p_gw;
	while(p_gw != NULL)
	{
		dev_info_t *p_dev = p_gw->p_dev;
		while(p_dev != NULL)
		{
			if(!memcmp(p_dev->zidentity_no, dc->zidentify_no, sizeof(zidentify_no_t)))
			{
				goto zdev_match;
			}

			p_dev = p_dev->next;
		}

		p_gw = p_gw->next;
	}

	send_ub_udp_respond(ipaddr, TRINFO_NONE, 
		dc->zidentify_no, dc->cidentify_no, NULL, 0);
	return;

zdev_match:
	send_dc_udp_request(p_gw->ipaddr, dc->tr_info, 
		dc->zidentify_no, dc->cidentify_no, dc->data, dc->data_len);
	return;
#endif
}


void send_dc_udp_request(char *ipaddr, tr_info_type_t trinfo,
	zidentify_no_t zidentify_no, cidentify_no_t cidentify_no, char *data, int len)
{
	dc_t dc;
	tr_buffer_t *buffer;

	memcpy(dc.zidentify_no, zidentify_no, sizeof(zidentify_no_t));
	memcpy(dc.cidentify_no, cidentify_no, sizeof(cidentify_no_t));
	dc.tr_info = trinfo;
	dc.data = data;
	dc.data_len = len;

	if((buffer = get_trbuffer_alloc(TRHEAD_DC, &dc)) == NULL)
	{
		return;
	}

	socket_udp_sendto(ipaddr, buffer->data, buffer->size);
	get_trbuffer_free(buffer);
}


void ub_handler(struct sockaddr_in *addr, ub_t *ub)
{
#ifdef COMM_SERVER
	cli_info_t *p_cli = get_client_list()->p_cli;
	while(p_cli != NULL)
	{
		if(!memcmp(p_cli->cidentify_no, ub->cidentify_no, sizeof(cidentify_no_t)))
		{
			goto client_match;
		}

		p_cli = p_cli->next;
	}

	return;

client_match:
	send_ub_udp_respond(p_cli->ipaddr, TRINFO_DATA, 
		ub->zidentify_no, ub->cidentify_no, ub->data, ub->data_len);
	return;
#endif
}


void send_ub_udp_respond(char *ipaddr, tr_info_type_t trinfo,
	zidentify_no_t zidentify_no, cidentify_no_t cidentify_no, char *data, int len)
{
	ub_t ub;
	tr_buffer_t *buffer;

	memcpy(ub.zidentify_no, zidentify_no, sizeof(zidentify_no_t));
	memcpy(ub.cidentify_no, cidentify_no, sizeof(cidentify_no_t));
	ub.tr_info = trinfo;
	ub.data = data;
	ub.data_len = len;

	if((buffer = get_trbuffer_alloc(TRHEAD_UB, &ub)) == NULL)
	{
		return;
	}

	socket_udp_sendto(ipaddr, buffer->data, buffer->size);
	get_trbuffer_free(buffer);
}


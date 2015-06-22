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
#include <module/balancer.h>

void pi_handler(struct sockaddr_in *addr, pi_t *pi)
{
	gw_info_t *p_gw = NULL;
	dev_info_t *p_dev = NULL;
	uint8 *buffer;

	char ipaddr[24] = {0};
	sprintf(ipaddr, "%s:%u", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
	
	switch(pi->trans_type)
	{
	case TRTYPE_UDP_NORMAL:
		switch(pi->fr_type)
		{
		case TRFRAME_CON:
#ifdef COMM_SERVER
			p_gw = query_gateway_info(pi->sn);

			if(p_gw == NULL)
			{
				pi_t mpi;
				memcpy(mpi.sn, pi->sn, sizeof(zidentify_no_t));
				mpi.trans_type = TRTYPE_UDP_NORMAL;
				mpi.fr_type = TRFRAME_GET;
				mpi.data = ipaddr;
				mpi.data_len = strlen(ipaddr);
				
				send_frame_udp_request(ipaddr, TRHEAD_PI, &mpi);
			}
			else
			{
				memset(p_gw->ipaddr, 0, sizeof(p_gw->ipaddr));
				memcpy(p_gw->ipaddr, ipaddr, strlen(ipaddr));

				int zdevs_nums = pi->data_len/4;
				if(zdevs_nums > 0 && zdevs_nums <= ZDEVICE_MAX_NUM)
				{
					p_dev = p_gw->p_dev;
					while(p_dev != NULL)
					{
						int i;
						for(i=0; i<zdevs_nums; i++)
						{
							uint16 znet_addr;
							incode_ctox16(&znet_addr, pi->data+(i<<2));
							if(p_dev->znet_addr == znet_addr)
							{
								goto next_zdev;
							}
						}

						del_zdev_info(p_gw, p_dev->znet_addr);
next_zdev:				p_dev = p_dev->next;
					}
				}
				else if(zdevs_nums == 0)
				{
					p_dev = p_gw->p_dev;
					while(p_dev != NULL)
					{
						del_zdev_info(p_gw, p_dev->znet_addr);
						p_dev = p_dev->next;
					}
				}
				
				set_gateway_check(p_gw->gw_no, p_gw->rand);
				
				bi_t bi;
				memcpy(bi.sn, pi->sn, sizeof(zidentify_no_t));
				bi.trans_type = TRTYPE_UDP_NORMAL;
				bi.fr_type = TRFRAME_REG;
				bi.data = ipaddr;
				bi.data_len = strlen(ipaddr);
				send_frame_udp_request(ipaddr, TRHEAD_BI, &bi);				
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

			memset(p_gw->serverip_addr, 0, sizeof(p_gw->serverip_addr));
			memcpy(p_gw->serverip_addr, ipaddr, strlen(ipaddr));
			p_gw->serverip_len = strlen(ipaddr);
			
			fr_buffer_t *frbuffer = get_gateway_buffer_alloc(p_gw);
			
			bi_t bi;
			memcpy(bi.sn, get_gateway_info()->gw_no, sizeof(zidentify_no_t));
			bi.trans_type = TRTYPE_UDP_NORMAL;
			bi.fr_type = TRFRAME_PUT_GW;
			bi.data = frbuffer->data;
			bi.data_len = frbuffer->size;
			send_frame_udp_request(ipaddr, TRHEAD_BI, &bi);
			
			get_buffer_free(frbuffer);

			p_dev = p_gw->p_dev;
			while(p_dev != NULL)
			{
				buffer = get_zdev_buffer_alloc(p_dev);
				
				bi_t zbi;
				memcpy(zbi.sn, get_gateway_info()->gw_no, sizeof(zidentify_no_t));
				zbi.trans_type = TRTYPE_UDP_NORMAL;
				zbi.fr_type = TRFRAME_PUT_DEV;
				zbi.data = buffer;
				zbi.data_len = ZDEVICE_BUFFER_SIZE;
				send_frame_udp_request(ipaddr, TRHEAD_BI, &zbi);
				
				get_zdev_buffer_free(buffer);
				p_dev = p_dev->next;
			}
#endif
			break;
		}
		break;

	case TRTYPE_TCP_LONG:
	case TRTYPE_TCP_SHORT:
		break;
	}
}

void bi_handler(struct sockaddr_in *addr, bi_t *bi)
{
	gw_info_t *p_gw = NULL;
	dev_info_t *p_dev = NULL;

	char ipaddr[24] = {0};
	sprintf(ipaddr, "%s:%u", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
	
	switch(bi->trans_type)
	{
	case TRTYPE_UDP_NORMAL:
		switch(bi->fr_type)
		{
		case TRFRAME_REG:
#ifdef COMM_CLIENT
			p_gw = get_gateway_info();
			p_gw->rand = gen_rand(bi->sn);
			memset(p_gw->ipaddr, 0, sizeof(p_gw->ipaddr));
			memcpy(p_gw->ipaddr, bi->data, bi->data_len);
			p_gw->ip_len = bi->data_len;
			
			memset(p_gw->serverip_addr, 0, sizeof(p_gw->serverip_addr));
			memcpy(p_gw->serverip_addr, ipaddr, strlen(ipaddr));
			p_gw->serverip_len = strlen(ipaddr);
#endif
			break;
			
		case TRFRAME_PUT_GW:
#ifdef COMM_SERVER
#ifdef LACK_EDTYPE_SUPPORT
			if((p_gw=get_old_gateway_frame_alloc(bi->data, bi->data_len)) == NULL)
			{
				break;
			}

			if(p_gw->ip_len < 8 || memcmp(bi->data+32, ipaddr, 8))
			{
				get_gateway_frame_free(p_gw);
				p_gw = NULL;
			}
#endif
			
			if(p_gw == NULL)
			{
				p_gw = get_gateway_frame_alloc(bi->data, bi->data_len);
			}
			
			if(p_gw != NULL)
			{
				p_gw->ip_len = strlen(ipaddr);
				memset(p_gw->ipaddr, 0, sizeof(p_gw->ipaddr));
				memcpy(p_gw->ipaddr, ipaddr, p_gw->ip_len);

				memset(p_gw->serverip_addr, 0, sizeof(p_gw->serverip_addr));
				GET_SERVER_IP(p_gw->serverip_addr);
				p_gw->serverip_len = strlen(p_gw->serverip_addr);
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
					get_devopt_data_free(p_dev->zdev_opt);
					get_zdev_frame_free(p_dev);
				}
			}
#endif
			break;
		}
		break;

	case TRTYPE_TCP_LONG:
	case TRTYPE_TCP_SHORT:
		break;
	}
}

void gp_handler(struct sockaddr_in *addr, gp_t *gp)
{
	char ipaddr[24] = {0};
	sprintf(ipaddr, "%s:%u", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
	fr_buffer_t *frbuffer = NULL;
	dev_info_t *p_dev = NULL;
	
#ifdef COMM_SERVER
	gp_t mgp;
	rp_t mrp, rp;
	uc_t m_uc;
	uo_t m_uo;
	
	cli_info_t *m_info = calloc(1, sizeof(cli_info_t));
	memcpy(m_info->cidentify_no, gp->cidentify_no, sizeof(cidentify_no_t));
	m_info->trans_type = gp->trans_type;
	memcpy(m_info->ipaddr, ipaddr, strlen(ipaddr));
	m_info->ip_len = strlen(ipaddr);
	GET_SERVER_IP(m_info->serverip_addr);
	m_info->serverip_len = strlen(m_info->serverip_addr);
	m_info->check_count = 0;
	m_info->check_conn = 1;
	m_info->next = NULL;
	
	if(add_client_info(m_info) != 0)
	{
		free(m_info);
	}

	gw_info_t *p_gw = get_gateway_list()->p_gw;
	while(p_gw != NULL)
	{
		if(!memcmp(p_gw->gw_no, gp->zidentify_no, sizeof(zidentify_no_t)))
		{
			goto gw_match;
		}
		
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

	memcpy(rp.zidentify_no, gp->zidentify_no, sizeof(zidentify_no_t));
	memcpy(rp.cidentify_no, gp->cidentify_no, sizeof(cidentify_no_t));
	rp.trans_type = TRTYPE_UDP_NORMAL;
	rp.tr_info = TRINFO_DATA;
	rp.data = NULL;
	rp.data_len = 0;
	send_frame_udp_request(ipaddr, TRHEAD_RP, &rp);
	
	return;

dev_match:	
	memcpy(mgp.zidentify_no, gp->zidentify_no, sizeof(zidentify_no_t));
	memcpy(mgp.cidentify_no, gp->cidentify_no, sizeof(cidentify_no_t));
	mgp.trans_type = TRTYPE_UDP_NORMAL;
	mgp.tr_info = TRINFO_IP;
	mgp.data = m_info->ipaddr;
	mgp.data_len = m_info->ip_len;
	send_frame_udp_request(p_gw->ipaddr, TRHEAD_GP, &mgp);

	memcpy(m_uo.head, FR_HEAD_UO, 3);
	m_uo.type = get_frnet_type_to_ch(p_dev->znet_type);
	get_frapp_type_to_str(m_uo.ed_type, p_dev->zapp_type);
	incode_xtocs(m_uo.ext_addr, p_dev->zidentity_no, 8);
	incode_xtoc16(m_uo.short_addr, p_dev->znet_addr);
	m_uo.data = NULL;
	m_uo.data_len = 0;
	memcpy(m_uo.tail, FR_TAIL, 4);
	
	frbuffer = get_buffer_alloc(HEAD_UO, &m_uo);

	memcpy(mrp.zidentify_no, p_gw->gw_no, sizeof(zidentify_no_t));
	memcpy(mrp.cidentify_no, gp->cidentify_no, sizeof(cidentify_no_t));
	mrp.trans_type = TRTYPE_UDP_NORMAL;
	mrp.tr_info = TRINFO_DATA;
	mrp.data = frbuffer->data;
	mrp.data_len = frbuffer->size;
	send_frame_udp_request(ipaddr, TRHEAD_RP, &mrp);

	get_buffer_free(frbuffer);
	return;

gw_match:
	memcpy(m_uc.head, FR_HEAD_UC, 3);
	m_uc.type = FR_DEV_ROUTER;
	get_frapp_type_to_str(m_uc.ed_type, p_gw->zapp_type);
	incode_xtoc16(m_uc.short_addr, 0);
	incode_xtocs(m_uc.ext_addr, p_gw->gw_no, 8);
	incode_xtoc16(m_uc.panid, p_gw->zpanid);
	incode_xtoc16(m_uc.channel, p_gw->zchannel);
	m_uc.data = NULL;
	m_uc.data_len = 0;
	memcpy(m_uc.tail, FR_TAIL, 4);
	
	frbuffer = get_switch_buffer_alloc(HEAD_UC, p_gw->zgw_opt, &m_uc);

	memcpy(mrp.zidentify_no, p_gw->gw_no, sizeof(zidentify_no_t));
	memcpy(mrp.cidentify_no, gp->cidentify_no, sizeof(cidentify_no_t));
	mrp.trans_type = TRTYPE_UDP_NORMAL;
	mrp.tr_info = TRINFO_DATA;
	mrp.data = frbuffer->data;
	mrp.data_len = frbuffer->size;
	send_frame_udp_request(ipaddr, TRHEAD_RP, &mrp);

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
	m_info->trans_type = gp->trans_type;
	memcpy(m_info->ipaddr, gp->data, gp->data_len);
	m_info->ip_len = gp->data_len;
	memcpy(m_info->serverip_addr, ipaddr, strlen(ipaddr));
	m_info->serverip_len = strlen(ipaddr);
	m_info->check_count = 3;
	m_info->check_conn = 1;
	m_info->next = NULL;
	
	if(add_client_info(m_info) != 0)
	{
		free(m_info);
	}
	
	if((p_dev=query_zdevice_info_with_sn(gp->zidentify_no)) == NULL)
	{
		return;
	}

	uo_t m_uo;
	memcpy(m_uo.head, FR_HEAD_UO, 3);
	m_uo.type = get_frnet_type_to_ch(p_dev->znet_type);
	get_frapp_type_to_str(m_uo.ed_type, p_dev->zapp_type);
	incode_xtocs(m_uo.ext_addr, p_dev->zidentity_no, 8);
	incode_xtoc16(m_uo.short_addr, p_dev->znet_addr);
	m_uo.data = NULL;
	m_uo.data_len = 0;
	memcpy(m_uo.tail, FR_TAIL, 4);
	
	frbuffer = get_buffer_alloc(HEAD_UO, &m_uo);
	
	rp_t mrp;
	memcpy(mrp.zidentify_no, get_gateway_info()->gw_no, sizeof(zidentify_no_t));
	memcpy(mrp.cidentify_no, gp->cidentify_no, sizeof(cidentify_no_t));
	mrp.trans_type = gp->trans_type;
	mrp.tr_info = TRINFO_UPDATE;
	mrp.data = frbuffer->data;
	mrp.data_len = frbuffer->size;
	send_frame_udp_request(ipaddr, TRHEAD_RP, &mrp);
	
	get_buffer_free(frbuffer);
	set_rp_check(query_client_info(gp->cidentify_no));
#endif
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
		cli_info_t *p_cli = query_client_info(rp->cidentify_no);
		p_cli->trans_type = rp->trans_type;
	}
#endif

#ifdef COMM_SERVER
	if(rp->tr_info == TRINFO_UPDATE)
	{
		cli_info_t *p_cli = query_client_info(rp->cidentify_no);
		if(p_cli != NULL)
		{
			send_frame_udp_request(p_cli->ipaddr, TRHEAD_RP, rp);
		}
	}
	else if(rp->tr_info == TRINFO_IP)
	{
		if(rp->data != NULL)
		{
			rp_t mrp;
			memcpy(mrp.zidentify_no, rp->zidentify_no, sizeof(zidentify_no_t));
			memcpy(mrp.cidentify_no, rp->cidentify_no, sizeof(cidentify_no_t));
			mrp.trans_type = TRTYPE_UDP_NORMAL;
			mrp.tr_info = TRINFO_IP;
			mrp.data = NULL;
			mrp.data_len = 0;
			send_frame_udp_request(rp->data, TRHEAD_RP, &mrp);
		}
	}
#endif

#ifdef CLIENT_TEST
	set_target_ip(ipaddr, strlen(ipaddr));

	rp_t mrp;
	memcpy(mrp.zidentify_no, rp->zidentify_no, sizeof(zidentify_no_t));
	memcpy(mrp.cidentify_no, rp->cidentify_no, sizeof(cidentify_no_t));
	mrp.trans_type = TRTYPE_UDP_NORMAL;
	mrp.tr_info = TRINFO_CUT;
	mrp.data = NULL;
	mrp.data_len = 0;
	send_frame_udp_request(ipaddr, TRHEAD_RP, &mrp);
#endif
}

void gd_handler(struct sockaddr_in *addr, gd_t *gd)
{
	char ipaddr[24] = {0};
	sprintf(ipaddr, "%s:%u", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));

#ifdef COMM_SERVER
	cli_info_t *m_info = calloc(1, sizeof(cli_info_t));
	memcpy(m_info->cidentify_no, gd->cidentify_no, sizeof(cidentify_no_t));
	m_info->trans_type = gd->trans_type;
	memcpy(m_info->ipaddr, ipaddr, strlen(ipaddr));
	m_info->ip_len = strlen(ipaddr);
	GET_SERVER_IP(m_info->serverip_addr);
	m_info->serverip_len = strlen(m_info->serverip_addr);
	m_info->check_count = 0;
	m_info->check_conn = 1;
	m_info->next = NULL;
	
	if(add_client_info(m_info) != 0)
	{
		free(m_info);
	}

	if(gd->tr_info != TRINFO_REG && gd->tr_info != TRINFO_HOLD)
	{
		return;
	}

	gw_info_t *p_gw = get_gateway_list()->p_gw;
	while(p_gw != NULL)
	{
		if(!memcmp(p_gw->gw_no, gd->zidentify_no, sizeof(zidentify_no_t)))
		{
			goto gdev_match;
		}

		p_gw = p_gw->next;
	}

	rd_t rd;
	memcpy(rd.zidentify_no, gd->zidentify_no, sizeof(zidentify_no_t));
	memcpy(rd.cidentify_no, gd->cidentify_no, sizeof(cidentify_no_t));
	rd.trans_type = TRTYPE_UDP_NORMAL;
	rd.tr_info = TRINFO_DISMATCH;
	rd.data = NULL;
	rd.data_len = 0;
	send_frame_udp_request(ipaddr, TRHEAD_RD, &rd);
	
	return;

gdev_match:
	if(gd->tr_info != TRINFO_HOLD)
	{
		gd_t mgd;
		memcpy(mgd.zidentify_no, gd->zidentify_no, sizeof(zidentify_no_t));
		memcpy(mgd.cidentify_no, gd->cidentify_no, sizeof(cidentify_no_t));
		mgd.trans_type = TRTYPE_UDP_NORMAL;
		mgd.tr_info = TRINFO_IP;
		mgd.data = ipaddr;
		mgd.data_len = strlen(ipaddr);
		send_frame_udp_request(p_gw->ipaddr, TRHEAD_GD, &mgd);
	}
	else
	{
		gd_t mgd;
		memcpy(mgd.zidentify_no, gd->zidentify_no, sizeof(zidentify_no_t));
		memcpy(mgd.cidentify_no, gd->cidentify_no, sizeof(cidentify_no_t));
		mgd.trans_type = TRTYPE_UDP_NORMAL;
		mgd.tr_info = TRINFO_HOLD;
		mgd.data = ipaddr;
		mgd.data_len = strlen(ipaddr);
		send_frame_udp_request(p_gw->ipaddr, TRHEAD_GD, &mgd);
	}

	dev_info_t *p_dev = p_gw->p_dev;
	char buffer[ZDEVICE_MAX_NUM<<4] = {0};
	char bsize = 0;
	while(p_dev != NULL && bsize < (ZDEVICE_MAX_NUM<<4))
	{
		incode_xtocs(buffer+bsize, p_dev->zidentity_no, 8);
		bsize += 16;
		p_dev = p_dev->next;
	}

	rd_t mrd;
	memcpy(mrd.zidentify_no, p_gw->gw_no, sizeof(zidentify_no_t));
	memcpy(mrd.cidentify_no, gd->cidentify_no, sizeof(cidentify_no_t));
	mrd.trans_type = TRTYPE_UDP_NORMAL;
	mrd.tr_info = TRINFO_FOUND;
	mrd.data = buffer;
	mrd.data_len = bsize;
	send_frame_udp_request(ipaddr, TRHEAD_RD, &mrd);
	
	return;
#endif
	
#ifdef COMM_CLIENT
	if(gd->data_len > IP_ADDR_MAX_SIZE)
	{
		return;
	}

	cli_info_t *m_info = calloc(1, sizeof(cli_info_t));

	switch(gd->tr_info)
	{
	case TRINFO_IP:
		memcpy(m_info->ipaddr, gd->data, gd->data_len);
		m_info->ip_len = gd->data_len;
		break;

	case TRINFO_HOLD:
	case TRINFO_REG:
		memcpy(m_info->ipaddr, ipaddr, strlen(ipaddr));
		m_info->ip_len = strlen(ipaddr);
		break;
	}
	
	memcpy(m_info->cidentify_no, gd->cidentify_no, sizeof(cidentify_no_t));	
	m_info->trans_type = gd->trans_type;
	GET_SERVER_IP(m_info->serverip_addr);
	m_info->serverip_len = strlen(m_info->serverip_addr);
	m_info->check_count = 0;
	m_info->check_conn = 1;
	m_info->next = NULL;
	
	if(add_client_info(m_info) != 0)
	{
		free(m_info);
	}

	set_cli_check(query_client_info(gd->cidentify_no));

	dev_info_t *p_dev = get_gateway_info()->p_dev;
	char buffer[ZDEVICE_MAX_NUM<<4] = {0};
	char bsize = 0;
	while(p_dev != NULL && bsize < (ZDEVICE_MAX_NUM<<4))
	{
		incode_xtocs(buffer+bsize, p_dev->zidentity_no, 8);
		bsize += 16;
		p_dev = p_dev->next;
	}

	if(gd->tr_info != TRINFO_HOLD)
	{
		gd_t mgd;
		memcpy(mgd.zidentify_no, get_gateway_info()->gw_no, sizeof(zidentify_no_t));
		memcpy(mgd.cidentify_no, gd->cidentify_no, sizeof(cidentify_no_t));
		mgd.trans_type = TRTYPE_UDP_TRAVERSAL;
		mgd.tr_info = TRINFO_REG;
		mgd.data = buffer;
		mgd.data_len = bsize;
		send_frame_udp_request(
			query_client_info(gd->cidentify_no)->ipaddr, TRHEAD_GD, &mgd);
	}

	return;
#endif


#ifdef CLIENT_TEST
	rd_t mrd;
	memcpy(mrd.zidentify_no, gd->zidentify_no, sizeof(zidentify_no_t));
	memcpy(mrd.cidentify_no, gd->cidentify_no, sizeof(cidentify_no_t));
	mrd.trans_type = TRTYPE_UDP_NORMAL;
	mrd.tr_info = TRINFO_NONE;
	mrd.data = NULL;
	mrd.data_len = 0;
	send_frame_udp_request(ipaddr, TRHEAD_RD, &mrd);
#endif
}

void rd_handler(struct sockaddr_in *addr, rd_t *rd)
{
	char ipaddr[24] = {0};
	sprintf(ipaddr, "%s:%u", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
	
#ifdef COMM_CLIENT
	cli_info_t *p_cli = query_client_info(rd->cidentify_no);

	p_cli->trans_type = rd->trans_type;
	p_cli->ip_len = strlen(ipaddr);
	memcpy(p_cli->ipaddr, ipaddr, p_cli->ip_len);
	
	set_cli_check(p_cli);
#endif
}

void dc_handler(struct sockaddr_in *addr, dc_t *dc)
{
	char ipaddr[24] = {0};
	sprintf(ipaddr, "%s:%u", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
	
#ifdef COMM_CLIENT
	cli_info_t *m_info = calloc(1, sizeof(cli_info_t));
	memcpy(m_info->cidentify_no, dc->cidentify_no, sizeof(cidentify_no_t));	
	m_info->trans_type = dc->trans_type;
	if(m_info->trans_type == TRTYPE_UDP_TRAVERSAL)
	{
		memcpy(m_info->ipaddr, ipaddr, strlen(ipaddr));
		m_info->ip_len = strlen(ipaddr);
	}
	else
	{
		memset(m_info->ipaddr, 0, sizeof(m_info->ipaddr));
		m_info->ip_len = 0;
	}
	GET_SERVER_IP(m_info->serverip_addr);
	m_info->serverip_len = strlen(m_info->serverip_addr);
	m_info->check_count = 0;
	m_info->check_conn = 1;
	m_info->next = NULL;
	
	if(add_client_info(m_info) != 0)
	{
		free(m_info);
	}

	fr_head_type_t head_type = get_frhead_from_str(dc->data);
	void *p = get_frame_alloc(head_type, dc->data, dc->data_len);
	if(p == NULL)
	{
		return;
	}

	switch(head_type)
	{
	case HEAD_UC:
	{
		uc_t *uc = (uc_t *)p;
		get_frame_free(HEAD_UC, uc);
	}
	break;
		
	case HEAD_UO:
	{
		uo_t *uo = (uo_t *)p;
		get_frame_free(HEAD_UO, uo);
	}
	break;
		
	case HEAD_UH:
	{
		uh_t *uh = (uh_t *)p;
		get_frame_free(HEAD_UH, uh);
	}
	break;
		
	case HEAD_UR:
	{
		ur_t *ur = (ur_t *)p;
		get_frame_free(HEAD_UR, ur);
	}
	break;
		
	case HEAD_DE:
	{
		fr_buffer_t *buffer = NULL;
		gw_info_t *p_mgw = get_gateway_info();
		dev_info_t *dev_info = NULL;
		de_t *de = (de_t *)p;
		if(memcmp(p_mgw->gw_no, dc->zidentify_no, sizeof(zidentify_no_t)))
		{
			dev_info = query_zdevice_info_with_sn(dc->zidentify_no);
		}
		else
		{
			set_devopt_fromstr(p_mgw->zgw_opt, de->data, de->data_len);
			buffer = get_switch_buffer_alloc(HEAD_DE, p_mgw->zgw_opt, de);
		}
		
		if(dev_info != NULL)
		{
			set_devopt_fromstr(dev_info->zdev_opt, de->data, de->data_len);

			if(dev_info->zdev_opt && 
				(dev_info->zdev_opt->type == FRAPP_DOOR_SENSOR
					|| dev_info->zdev_opt->type == FRAPP_IR_DETECTION))
			{
				ur_t ur;
				memcpy(ur.head, FR_HEAD_UR, 3);
				ur.type = get_frnet_type_to_ch(dev_info->znet_type);
				get_frapp_type_to_str(ur.ed_type, dev_info->zapp_type);
				incode_xtoc16(ur.short_addr, dev_info->znet_addr);
				ur.data_len = de->data_len;
				ur.data = de->data;
				memcpy(ur.tail, FR_TAIL, 4);
									
				fr_buffer_t *frbuffer = get_switch_buffer_alloc(HEAD_UR, 
										dev_info->zdev_opt, &ur);
		
				cli_info_t *p_cli = get_client_list()->p_cli;
				while(p_cli != NULL)
				{
					ub_t ub;
					memcpy(ub.zidentify_no, get_gateway_info()->gw_no, sizeof(zidentify_no_t));
					memcpy(ub.cidentify_no, p_cli->cidentify_no, sizeof(cidentify_no_t));
					ub.trans_type = p_cli->trans_type;
					ub.tr_info = TRINFO_REDATA;
					ub.data = frbuffer->data;
					ub.data_len = frbuffer->size;
					if(p_cli->trans_type == TRTYPE_UDP_NORMAL)
					{
						send_frame_udp_request(p_cli->serverip_addr, TRHEAD_UB, &ub);
					}
					else if(p_cli->trans_type == TRTYPE_UDP_TRAVERSAL)
					{
						send_frame_udp_request(p_cli->ipaddr, TRHEAD_UB, &ub);
					}

					p_cli = p_cli->next;
				}
				get_buffer_free(frbuffer);
				
				goto Handle_UR_free;
			}
			
			buffer = get_switch_buffer_alloc(HEAD_DE, 
										dev_info->zdev_opt, de);
		}

		if(buffer != NULL)
		{
			serial_write(buffer->data, buffer->size);
			get_buffer_free(buffer);
		}
Handle_UR_free:
		get_frame_free(HEAD_DE, de);
	}
	break;
		
	default: break;
	}
	
	return;
#endif

#ifdef COMM_SERVER
	dc_t mdc;
	gw_info_t *p_gw = get_gateway_list()->p_gw;
	while(p_gw != NULL)
	{
		if(!memcmp(p_gw->gw_no, dc->zidentify_no, sizeof(zidentify_no_t)))
		{
			goto gwdev_match;
		}
		
		dev_info_t *p_dev = p_gw->p_dev;
		while(p_dev != NULL)
		{
			if(!memcmp(p_dev->zidentity_no, dc->zidentify_no, sizeof(zidentify_no_t)))
			{
				goto gwdev_match;
			}

			p_dev = p_dev->next;
		}

		p_gw = p_gw->next;
	}

	ub_t ub;
	memcpy(ub.zidentify_no, dc->zidentify_no, sizeof(zidentify_no_t));
	memcpy(ub.cidentify_no, dc->cidentify_no, sizeof(cidentify_no_t));
	ub.trans_type = TRTYPE_UDP_NORMAL;
	ub.tr_info = TRINFO_DISMATCH;
	ub.data = NULL;
	ub.data_len = 0;
	send_frame_udp_request(ipaddr, TRHEAD_UB, &ub);
	
	return;

gwdev_match:
	memcpy(mdc.zidentify_no, dc->zidentify_no, sizeof(zidentify_no_t));
	memcpy(mdc.cidentify_no, dc->cidentify_no, sizeof(cidentify_no_t));
	mdc.trans_type = TRTYPE_UDP_NORMAL;
	mdc.tr_info = dc->tr_info;
	mdc.data = dc->data;
	mdc.data_len = dc->data_len;
	send_frame_udp_request(p_gw->ipaddr, TRHEAD_DC, &mdc);
	return;
#endif
}

void ub_handler(struct sockaddr_in *addr, ub_t *ub)
{
#ifdef COMM_SERVER
	ub_t mub;
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
	memcpy(mub.zidentify_no, ub->zidentify_no, sizeof(zidentify_no_t));
	memcpy(mub.cidentify_no, ub->cidentify_no, sizeof(cidentify_no_t));
	mub.trans_type = TRTYPE_UDP_NORMAL;
	mub.tr_info = ub->tr_info;
	mub.data = ub->data;
	mub.data_len = ub->data_len;
	send_frame_udp_request(p_cli->ipaddr, TRHEAD_UB, &mub);
	return;
#endif
}

void send_frame_udp_request(char *ipaddr, tr_head_type_t htype, void *frame)
{
	tr_buffer_t *buffer;
	
	switch(htype)
	{
	case TRHEAD_PI: 
	case TRHEAD_BI: 
	case TRHEAD_GP:
	case TRHEAD_RP:
	case TRHEAD_GD:
	case TRHEAD_RD:
	case TRHEAD_DC:
	case TRHEAD_UB:
	{
		if((buffer = get_trbuffer_alloc(htype, frame)) == NULL)
		{
			return;
		}

		socket_udp_sendto(ipaddr, buffer->data, buffer->size);
		get_trbuffer_free(buffer);	
	}
	break;
	}
}
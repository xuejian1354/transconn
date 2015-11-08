/*
 * request.c
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
#include "request.h"
#include <protocol/protocol.h>
#include <module/balancer.h>
#include <module/dbopt.h>

void pi_handler(struct sockaddr_in *addr, pi_t *p_pi)
{
	gw_info_t *p_gw = NULL;
	dev_info_t *p_dev = NULL;
	uint8 *buffer;

	char ipaddr[24] = {0};
	sprintf(ipaddr, "%s:%u", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
	
	switch(p_pi->trans_type)
	{
	case TRTYPE_UDP_NORMAL:
	case TRTYPE_UDP_TRANS:
		switch(p_pi->fr_type)
		{
		case TRFRAME_CON:
#ifdef COMM_SERVER
			p_gw = query_gateway_info(p_pi->sn);

			if(p_gw == NULL)
			{
				pi_t mpi;
				memcpy(mpi.sn, p_pi->sn, sizeof(zidentify_no_t));
				mpi.trans_type = p_pi->trans_type;
				mpi.fr_type = TRFRAME_GET;
				mpi.data = ipaddr;
				mpi.data_len = strlen(ipaddr);

				enable_datalog_atime();
				send_frame_udp_request(ipaddr, TRHEAD_PI, &mpi);
			}
			else
			{
				p_gw->trans_type = p_pi->trans_type;
				memset(p_gw->ipaddr, 0, sizeof(p_gw->ipaddr));
				memcpy(p_gw->ipaddr, ipaddr, strlen(ipaddr));

				int zdevs_nums = p_pi->data_len/4;
#ifdef DB_API_SUPPORT
				int online_nums = 0;
				int offline_nums = 0;
				uint16 online_addrs[ZDEVICE_MAX_NUM] = {0};
				uint16 offline_addrs[ZDEVICE_MAX_NUM] = {0};
#endif
				if(zdevs_nums > 0 && zdevs_nums <= ZDEVICE_MAX_NUM)
				{
					p_dev = p_gw->p_dev;
					while(p_dev != NULL)
					{
						int i;
						for(i=0; i<zdevs_nums; i++)
						{
							uint16 znet_addr;
							incode_ctox16(&znet_addr, p_pi->data+(i<<2));
							if(p_dev->znet_addr == znet_addr)
							{
#ifdef DB_API_SUPPORT
								online_addrs[online_nums++] = p_dev->znet_addr;
#endif
								goto next_zdev;
							}
						}
#ifdef DB_API_SUPPORT
						offline_addrs[offline_nums++] = p_dev->znet_addr;
#endif
						del_zdev_info(p_gw, p_dev->znet_addr);
next_zdev:				p_dev = p_dev->next;
					}

#ifdef DB_API_SUPPORT
					sql_uponline_zdev(p_gw, 0, offline_addrs, offline_nums);
					sql_uponline_zdev(p_gw, 1, online_addrs, online_nums);
#endif
				}
				else if(zdevs_nums == 0)
				{
					p_dev = p_gw->p_dev;
					while(p_dev != NULL)
					{
#ifdef DB_API_SUPPORT
						offline_addrs[offline_nums++] = p_dev->znet_addr;
#endif
						del_zdev_info(p_gw, p_dev->znet_addr);
						p_dev = p_dev->next;
					}
#ifdef DB_API_SUPPORT
					sql_uponline_zdev(p_gw, 0, offline_addrs, offline_nums);
#endif
				}
				
				set_gateway_check(p_gw->gw_no, p_gw->rand);
				
				bi_t bi;
				memcpy(bi.sn, p_pi->sn, sizeof(zidentify_no_t));
				bi.trans_type = p_pi->trans_type;
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
			p_gw->rand = gen_rand(p_pi->sn);
			memset(p_gw->ipaddr, 0, sizeof(p_gw->ipaddr));
			memcpy(p_gw->ipaddr, p_pi->data, p_pi->data_len);
			p_gw->ip_len = p_pi->data_len;

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
			enable_datalog_atime();
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
				enable_datalog_atime();
				send_frame_udp_request(ipaddr, TRHEAD_BI, &zbi);
				
				get_zdev_buffer_free(buffer);
				p_dev = p_dev->next;
			}
#endif
			break;

		case TRFRAME_TRANS:
#ifdef COMM_SERVER
			p_gw = query_gateway_info(p_pi->sn);

			if(p_gw == NULL)
			{
				pi_t mpi;
				memcpy(mpi.sn, p_pi->sn, sizeof(zidentify_no_t));
				mpi.trans_type = p_pi->trans_type;
				mpi.fr_type = TRFRAME_GET;
				mpi.data = ipaddr;
				mpi.data_len = strlen(ipaddr);

				enable_datalog_atime();
				send_frame_udp_request(ipaddr, TRHEAD_PI, &mpi);
			}
			else
			{
				p_gw->trans_type = p_pi->trans_type;
				memset(p_gw->ipaddr, 0, sizeof(p_gw->ipaddr));
				memcpy(p_gw->ipaddr, ipaddr, strlen(ipaddr));

				set_gateway_check(p_gw->gw_no, p_gw->rand);
				
				bi_t bi;
				memcpy(bi.sn, p_pi->sn, sizeof(zidentify_no_t));
				bi.trans_type = p_pi->trans_type;
				bi.fr_type = TRFRAME_REG;
				bi.data = ipaddr;
				bi.data_len = strlen(ipaddr);
				send_frame_udp_request(ipaddr, TRHEAD_BI, &bi);				
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

void bi_handler(struct sockaddr_in *addr, bi_t *p_bi)
{
	gw_info_t *p_gw = NULL;
	dev_info_t *p_dev = NULL;

	char ipaddr[24] = {0};
	sprintf(ipaddr, "%s:%u", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
	
	switch(p_bi->trans_type)
	{
	case TRTYPE_UDP_NORMAL:
	case TRTYPE_UDP_TRANS:
		switch(p_bi->fr_type)
		{
		case TRFRAME_REG:
#ifdef COMM_CLIENT
			p_gw = get_gateway_info();
			p_gw->rand = gen_rand(p_bi->sn);
			memset(p_gw->ipaddr, 0, sizeof(p_gw->ipaddr));
			memcpy(p_gw->ipaddr, p_bi->data, p_bi->data_len);
			p_gw->ip_len = p_bi->data_len;
			
			memset(p_gw->serverip_addr, 0, sizeof(p_gw->serverip_addr));
			memcpy(p_gw->serverip_addr, ipaddr, strlen(ipaddr));
			p_gw->serverip_len = strlen(ipaddr);
#endif
			break;
			
		case TRFRAME_PUT_GW:
#ifdef COMM_SERVER
#ifdef LACK_EDTYPE_SUPPORT
			if((p_gw=get_old_gateway_frame_alloc(p_bi->data, p_bi->data_len)) == NULL)
			{
				break;
			}

			if(p_gw->ip_len < 8 || memcmp(p_bi->data+32, ipaddr, 8))
			{
				get_gateway_frame_free(p_gw);
				p_gw = NULL;
			}
#endif
			
			if(p_gw == NULL)
			{
				p_gw = get_gateway_frame_alloc(p_bi->data, p_bi->data_len);
			}
			
			if(p_gw != NULL)
			{
				p_gw->trans_type = p_bi->trans_type;
				p_gw->ip_len = strlen(ipaddr);
				memset(p_gw->ipaddr, 0, sizeof(p_gw->ipaddr));
				memcpy(p_gw->ipaddr, ipaddr, p_gw->ip_len);

				memset(p_gw->serverip_addr, 0, sizeof(p_gw->serverip_addr));
				GET_SERVER_IP(p_gw->serverip_addr);
				p_gw->serverip_len = strlen(p_gw->serverip_addr);
				set_gateway_check(p_gw->gw_no, p_gw->rand);
#ifdef DB_API_SUPPORT
				pthread_mutex_lock(get_sql_add_lock());
				if(sql_add_gateway(p_gw) != 0)
#else
				if(add_gateway_info(p_gw) != 0)
#endif
				{
					get_gateway_frame_free(p_gw);
				}
#ifdef DB_API_SUPPORT
				pthread_mutex_unlock(get_sql_add_lock());
#endif
			}
#endif
			break;

		case TRFRAME_PUT_DEV:
#ifdef COMM_SERVER
			if((p_gw=query_gateway_info(p_bi->sn)) != NULL)
			{
				p_dev = get_zdev_frame_alloc(p_bi->data, p_bi->data_len);
#ifdef DB_API_SUPPORT
				pthread_mutex_lock(get_sql_add_lock());
				if(p_dev != NULL && sql_add_zdev(p_gw, p_dev) != 0)
#else
				if(p_dev != NULL && add_zdev_info(p_gw, p_dev) != 0)
#endif
				{
					get_devopt_data_free(p_dev->zdev_opt);
					get_zdev_frame_free(p_dev);
				}
#ifdef DB_API_SUPPORT
				pthread_mutex_unlock(get_sql_add_lock());
#endif
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

void ul_handler(struct sockaddr_in *addr, ul_t *p_ul)
{
	char ipaddr[24] = {0};
	sprintf(ipaddr, "%s:%u", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
	
#ifdef COMM_SERVER
	cli_info_t *m_info = calloc(1, sizeof(cli_info_t));
	memcpy(m_info->cidentify_no, p_ul->sn, sizeof(cidentify_no_t));
	m_info->trans_type = TRTYPE_NONE;
	memcpy(m_info->ipaddr, ipaddr, strlen(ipaddr));
	m_info->ip_len = strlen(ipaddr);
	GET_SERVER_IP(m_info->serverip_addr);
	m_info->serverip_len = strlen(m_info->serverip_addr);
	m_info->check_count = 0;
	m_info->check_conn = 1;
	m_info->user_info = NULL;
	m_info->next = NULL;
	
	if(add_client_info(m_info) != 0)
	{
		free(m_info);
	}

	cli_info_t *t_info = query_client_info(p_ul->sn);
	if(t_info != NULL && p_ul->data_len > 0)
	{
		memcpy(t_info->email, p_ul->data, p_ul->data_len);
#ifdef DB_API_SUPPORT
		t_info->user_info = calloc(1, sizeof(cli_user_t));
		set_user_info_from_sql(t_info->email, t_info->user_info);

		sl_t sl;
		memcpy(sl.sn, p_ul->sn, sizeof(zidentify_no_t));
		sl.trans_type = TRTYPE_UDP_NORMAL;
		sl.fr_type = TRFRAME_IGNORE;

		if(strlen(t_info->user_info->name) > 0)
		{
			sl.data = t_info->user_info->name;
			sl.data_len = strlen(t_info->user_info->name);
			enable_datalog_atime();
			send_frame_udp_request(ipaddr, TRHEAD_SL, &sl);
		}

		if(t_info->user_info->devices != NULL)
		{
			sl.data = t_info->user_info->devices;
			sl.data_len = strlen(t_info->user_info->devices);
			enable_datalog_atime();
			send_frame_udp_request(ipaddr, TRHEAD_SL, &sl);
			free(t_info->user_info->devices);
			t_info->user_info->devices = NULL;
		}

		if(t_info->user_info->areas != NULL)
		{
			sl.data = t_info->user_info->areas;
			sl.data_len = strlen(t_info->user_info->areas);
			enable_datalog_atime();
			send_frame_udp_request(ipaddr, TRHEAD_SL, &sl);
			free(t_info->user_info->areas);
			t_info->user_info->areas = NULL;
		}

		if(t_info->user_info->scenes != NULL)
		{
			sl.data = t_info->user_info->scenes;
			sl.data_len = strlen(t_info->user_info->scenes);
			enable_datalog_atime();
			send_frame_udp_request(ipaddr, TRHEAD_SL, &sl);
			free(t_info->user_info->scenes);
			t_info->user_info->scenes = NULL;
		}
		
		free(t_info->user_info);
		t_info->user_info = NULL;
#endif
	}
#endif
}

void sl_handler(struct sockaddr_in *addr, sl_t *p_sl)
{}

void gp_handler(struct sockaddr_in *addr, gp_t *p_gp)
{
	char ipaddr[24] = {0};
	sprintf(ipaddr, "%s:%u", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
	fr_buffer_t *frbuffer = NULL;
	dev_info_t *p_dev = NULL;
	
#ifdef COMM_SERVER
	rp_t mrp, rp;
	uc_t m_uc;
	uo_t m_uo;
	
	cli_info_t *m_info = calloc(1, sizeof(cli_info_t));
	memcpy(m_info->cidentify_no, p_gp->cidentify_no, sizeof(cidentify_no_t));
	m_info->trans_type = p_gp->trans_type;
	memcpy(m_info->ipaddr, ipaddr, strlen(ipaddr));
	m_info->ip_len = strlen(ipaddr);
	GET_SERVER_IP(m_info->serverip_addr);
	m_info->serverip_len = strlen(m_info->serverip_addr);
	m_info->check_count = 0;
	m_info->check_conn = 1;
	m_info->user_info = NULL;
	m_info->next = NULL;
	
	if(add_client_info(m_info) != 0)
	{
		free(m_info);
	}

	if(!memcmp(p_gp->zidentify_no, get_broadcast_no(), sizeof(zidentify_no_t)))
	{
		return;
	}

	gw_info_t *p_gw = get_gateway_list()->p_gw;
	while(p_gw != NULL)
	{
		if(!memcmp(p_gw->gw_no, p_gp->zidentify_no, sizeof(zidentify_no_t)))
		{
			goto gw_match;
		}
		
		p_dev = p_gw->p_dev;
		while(p_dev != NULL)
		{
			if(!memcmp(p_dev->zidentity_no, p_gp->zidentify_no, sizeof(zidentify_no_t)))
			{
				goto dev_match;
			}

			p_dev = p_dev->next;
		}

		p_gw = p_gw->next;
	}

	memcpy(rp.zidentify_no, p_gp->zidentify_no, sizeof(zidentify_no_t));
	memcpy(rp.cidentify_no, p_gp->cidentify_no, sizeof(cidentify_no_t));
	rp.trans_type = TRTYPE_UDP_NORMAL;
	rp.tr_info = TRINFO_DATA;
	rp.data = NULL;
	rp.data_len = 0;
	enable_datalog_atime();
	send_frame_udp_request(ipaddr, TRHEAD_RP, &rp);
	
	return;

dev_match:	
#ifdef DB_API_SUPPORT
	if(1)
	{
		cli_info_t *t_cli = query_client_info(p_gp->cidentify_no);
		if(t_cli != NULL && strlen(t_cli->email) > 0)
		{
			set_zdev_to_user_sql(t_cli->email, p_dev->zidentity_no);
		}
	}
#endif
	
	if(p_gw->trans_type != TRTYPE_UDP_TRANS)
	{
		gp_t mgp;
		memcpy(mgp.zidentify_no, p_gp->zidentify_no, sizeof(zidentify_no_t));
		memcpy(mgp.cidentify_no, p_gp->cidentify_no, sizeof(cidentify_no_t));
		mgp.trans_type = TRTYPE_UDP_NORMAL;
		mgp.tr_info = TRINFO_IP;
		mgp.data = m_info->ipaddr;
		mgp.data_len = m_info->ip_len;
		enable_datalog_atime();
		send_frame_udp_request(p_gw->ipaddr, TRHEAD_GP, &mgp);
	}

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
	memcpy(mrp.cidentify_no, p_gp->cidentify_no, sizeof(cidentify_no_t));
	mrp.trans_type = TRTYPE_UDP_NORMAL;
	mrp.tr_info = TRINFO_DATA;
	mrp.data = frbuffer->data;
	mrp.data_len = frbuffer->size;
	enable_datalog_atime();
	send_frame_udp_request(ipaddr, TRHEAD_RP, &mrp);

	get_buffer_free(frbuffer);
	return;

gw_match:
#ifdef DB_API_SUPPORT
	if(1)
	{
		cli_info_t *t_cli = query_client_info(p_gp->cidentify_no);
		if(t_cli != NULL && strlen(t_cli->email) > 0)
		{
			set_zdev_to_user_sql(t_cli->email, p_gw->gw_no);
		}
	}
#endif
	
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
	memcpy(mrp.cidentify_no, p_gp->cidentify_no, sizeof(cidentify_no_t));
	mrp.trans_type = TRTYPE_UDP_NORMAL;
	mrp.tr_info = TRINFO_DATA;
	mrp.data = frbuffer->data;
	mrp.data_len = frbuffer->size;
	enable_datalog_atime();
	send_frame_udp_request(ipaddr, TRHEAD_RP, &mrp);
	
	get_buffer_free(frbuffer);
	return;
#endif

#ifdef COMM_CLIENT
	if(p_gp->data_len > IP_ADDR_MAX_SIZE)
	{
		return;
	}
	
	cli_info_t *m_info = calloc(1, sizeof(cli_info_t));
	memcpy(m_info->cidentify_no, p_gp->cidentify_no, sizeof(cidentify_no_t));
	m_info->trans_type = p_gp->trans_type;
	m_info->check_count = 3;
	m_info->check_conn = 1;
	m_info->user_info = NULL;
	m_info->next = NULL;
	
	if(p_gp->tr_info == TRINFO_IP)
	{
		
		memcpy(m_info->ipaddr, p_gp->data, p_gp->data_len);
		m_info->ip_len = p_gp->data_len;
		memcpy(m_info->serverip_addr, ipaddr, strlen(ipaddr));
		m_info->serverip_len = strlen(ipaddr);
	}
	else
	{
		memcpy(m_info->ipaddr, ipaddr, strlen(ipaddr));
		m_info->ip_len = strlen(ipaddr);
		GET_SERVER_IP(m_info->serverip_addr);
		m_info->serverip_len = strlen(m_info->serverip_addr);
	}
	
	if(add_client_info(m_info) != 0)
	{
		free(m_info);
	}
	
	if(!memcmp(p_gp->zidentify_no, get_broadcast_no(), sizeof(zidentify_no_t)))
	{
		rp_t mrp;
		memcpy(mrp.zidentify_no, get_gateway_info()->gw_no, sizeof(zidentify_no_t));
		memcpy(mrp.cidentify_no, p_gp->cidentify_no, sizeof(cidentify_no_t));
		mrp.trans_type = p_gp->trans_type;
		mrp.tr_info = TRINFO_UPDATE;

		int isFound = 0;
		dev_info_t *t_dev = get_gateway_info()->p_dev;
		while(t_dev != NULL)
		{
			uo_t m_uo;
			memcpy(m_uo.head, FR_HEAD_UO, 3);
			m_uo.type = get_frnet_type_to_ch(t_dev->znet_type);
			get_frapp_type_to_str(m_uo.ed_type, t_dev->zapp_type);
			incode_xtocs(m_uo.ext_addr, t_dev->zidentity_no, 8);
			incode_xtoc16(m_uo.short_addr, t_dev->znet_addr);
			m_uo.data = NULL;
			m_uo.data_len = 0;
			memcpy(m_uo.tail, FR_TAIL, 4);
			
			frbuffer = get_buffer_alloc(HEAD_UO, &m_uo);
			mrp.data = frbuffer->data;
			mrp.data_len = frbuffer->size;
			enable_datalog_atime();
			send_frame_udp_request(ipaddr, TRHEAD_RP, &mrp);

			char serverip_addr[24] = {0};
			GET_SERVER_IP(serverip_addr);
			if(memcmp(serverip_addr, ipaddr, strlen(ipaddr)))
			{
				send_frame_udp_request(serverip_addr, TRHEAD_RP, &mrp);	
			}

			get_buffer_free(frbuffer);

			usleep(1500);
			isFound = 1;
			t_dev = t_dev->next;
		}

		if(isFound)
		{
			mrp.tr_info = TRINFO_FOUND;
		}
		else
		{
			mrp.tr_info = TRINFO_DISMATCH;
		}
		
		mrp.data = NULL;
		mrp.data_len = 0;
		enable_datalog_atime();
		send_frame_udp_request(ipaddr, TRHEAD_RP, &mrp);
		set_rp_check(query_client_info(p_gp->cidentify_no));
		return;
	}
	else if((p_dev=query_zdevice_info_with_sn(p_gp->zidentify_no)) != NULL)
	{
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
		memcpy(mrp.cidentify_no, p_gp->cidentify_no, sizeof(cidentify_no_t));
		mrp.trans_type = p_gp->trans_type;
		mrp.tr_info = TRINFO_UPDATE;
		mrp.data = frbuffer->data;
		mrp.data_len = frbuffer->size;
		enable_datalog_atime();
		send_frame_udp_request(ipaddr, TRHEAD_RP, &mrp);
		
		get_buffer_free(frbuffer);
		set_rp_check(query_client_info(p_gp->cidentify_no));
		return;
	}

#endif
}

void rp_handler(struct sockaddr_in *addr, rp_t *p_rp)
{
	char ipaddr[24] = {0};
	sprintf(ipaddr, "%s:%u", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
	
#ifdef COMM_CLIENT
	if(memcmp(get_gateway_info()->gw_no, p_rp->zidentify_no, sizeof(zidentify_no_t))
		|| p_rp->data_len > IP_ADDR_MAX_SIZE)
	{
		return;
	}

	if(p_rp->tr_info == TRINFO_CUT)
	{
		set_rp_check(NULL);
		cli_info_t *p_cli = query_client_info(p_rp->cidentify_no);
		p_cli->trans_type = p_rp->trans_type;
	}
#endif

#ifdef COMM_SERVER
	if(p_rp->tr_info == TRINFO_UPDATE)
	{
		cli_info_t *p_cli = query_client_info(p_rp->cidentify_no);
		if(p_cli != NULL)
		{
			//enable_datalog_atime();
			//send_frame_udp_request(p_cli->ipaddr, TRHEAD_RP, p_rp);
#ifdef DB_API_SUPPORT
			if(strlen(p_cli->email) > 0)
			{
				uo_t *p_uo = get_frame_alloc(HEAD_UO, p_rp->data, p_rp->data_len);

				if(p_uo == NULL)
				{
					return;
				}

				uint8 exts[18] = {0};
				memcpy(exts, p_uo->ext_addr, 16);
				set_zdev_to_user_sql(p_cli->email, exts);

				get_frame_free(HEAD_UO, p_uo);
			}
#endif
		}
	}
	else if(p_rp->tr_info == TRINFO_IP)
	{
		if(p_rp->data != NULL)
		{
			rp_t mrp;
			memcpy(mrp.zidentify_no, p_rp->zidentify_no, sizeof(zidentify_no_t));
			memcpy(mrp.cidentify_no, p_rp->cidentify_no, sizeof(cidentify_no_t));
			mrp.trans_type = TRTYPE_UDP_NORMAL;
			mrp.tr_info = TRINFO_IP;
			mrp.data = NULL;
			mrp.data_len = 0;
			enable_datalog_atime();
			send_frame_udp_request(p_rp->data, TRHEAD_RP, &mrp);
		}
	}
#endif

#ifdef CLIENT_TEST
	set_target_ip(ipaddr, strlen(ipaddr));

	rp_t mrp;
	memcpy(mrp.zidentify_no, p_rp->zidentify_no, sizeof(zidentify_no_t));
	memcpy(mrp.cidentify_no, p_rp->cidentify_no, sizeof(cidentify_no_t));
	mrp.trans_type = TRTYPE_UDP_NORMAL;
	mrp.tr_info = TRINFO_CUT;
	mrp.data = NULL;
	mrp.data_len = 0;
	send_frame_udp_request(ipaddr, TRHEAD_RP, &mrp);
#endif
}

void gd_handler(struct sockaddr_in *addr, gd_t *p_gd)
{
	char ipaddr[24] = {0};
	sprintf(ipaddr, "%s:%u", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));

#ifdef COMM_SERVER
	cli_info_t *m_info = calloc(1, sizeof(cli_info_t));
	memcpy(m_info->cidentify_no, p_gd->cidentify_no, sizeof(cidentify_no_t));
	m_info->trans_type = p_gd->trans_type;
	memcpy(m_info->ipaddr, ipaddr, strlen(ipaddr));
	m_info->ip_len = strlen(ipaddr);
	GET_SERVER_IP(m_info->serverip_addr);
	m_info->serverip_len = strlen(m_info->serverip_addr);
	m_info->check_count = 0;
	m_info->check_conn = 1;
	m_info->user_info = NULL;
	m_info->next = NULL;
	
	if(add_client_info(m_info) != 0)
	{
		free(m_info);
	}

	if(p_gd->tr_info != TRINFO_REG && p_gd->tr_info != TRINFO_HOLD)
	{
		return;
	}

	gw_info_t *p_gw = get_gateway_list()->p_gw;
	while(p_gw != NULL)
	{
		if(!memcmp(p_gw->gw_no, p_gd->zidentify_no, sizeof(zidentify_no_t)))
		{
			goto gdev_match;
		}

		dev_info_t *p_dev = p_gw->p_dev;
		while(p_dev != NULL)
		{
			if(!memcmp(p_dev->zidentity_no, p_gd->zidentify_no, sizeof(zidentify_no_t)))
			{
				goto gdev_match;
			}

			p_dev = p_dev->next;
		}

		p_gw = p_gw->next;
	}

	rd_t rd;
	memcpy(rd.zidentify_no, p_gd->zidentify_no, sizeof(zidentify_no_t));
	memcpy(rd.cidentify_no, p_gd->cidentify_no, sizeof(cidentify_no_t));
	rd.trans_type = TRTYPE_UDP_NORMAL;
	rd.tr_info = TRINFO_DISMATCH;
	rd.data = NULL;
	rd.data_len = 0;
	send_frame_udp_request(ipaddr, TRHEAD_RD, &rd);
	
	return;

gdev_match:
	/*if(p_gw->trans_type != TRTYPE_UDP_TRANS)
	{
		gd_t mgd;
		memcpy(mgd.zidentify_no, p_gd->zidentify_no, sizeof(zidentify_no_t));
		memcpy(mgd.cidentify_no, p_gd->cidentify_no, sizeof(cidentify_no_t));
		mgd.trans_type = TRTYPE_UDP_NORMAL;
		if(p_gd->tr_info != TRINFO_HOLD)
		{
			mgd.tr_info = TRINFO_IP;
		}
		else
		{
			mgd.tr_info = TRINFO_HOLD;
		}
		mgd.data = ipaddr;
		mgd.data_len = strlen(ipaddr);
		send_frame_udp_request(p_gw->ipaddr, TRHEAD_GD, &mgd);
	}*/

	if(p_gw->trans_type == TRTYPE_UDP_TRANS)
	{
		cli_info_t *p_cli = query_client_info(p_gd->cidentify_no);
		cli_contain_t *m_contain = calloc(1, sizeof(cli_contain_t));
		m_contain->p_cli = p_cli;
		if(add_contain_info(&p_gw->p_contain, m_contain) != 0)
		{
			free(m_contain);
		}

		rd_t crd;
		memcpy(crd.zidentify_no, p_gd->zidentify_no, sizeof(zidentify_no_t));
		memcpy(crd.cidentify_no, p_gd->cidentify_no, sizeof(cidentify_no_t));
		crd.trans_type = TRTYPE_UDP_NORMAL;
		crd.tr_info = TRINFO_FOUND;
		crd.data = NULL;
		crd.data_len = 0;
		send_frame_udp_request(ipaddr, TRHEAD_RD, &crd);
		return;
	}

	dev_info_t *p_dev = p_gw->p_dev;
	char buffer[ZDEVICE_MAX_NUM<<2] = {0};
	char bsize = 0;
	while(p_dev != NULL && bsize < (ZDEVICE_MAX_NUM<<2))
	{
		incode_xtoc16(buffer+bsize, p_dev->znet_addr);
		bsize += 4;
		p_dev = p_dev->next;
	}

	rd_t mrd;
	memcpy(mrd.zidentify_no, p_gw->gw_no, sizeof(zidentify_no_t));
	memcpy(mrd.cidentify_no, p_gd->cidentify_no, sizeof(cidentify_no_t));
	mrd.trans_type = TRTYPE_UDP_NORMAL;
	mrd.tr_info = TRINFO_FOUND;
	mrd.data = buffer;
	mrd.data_len = bsize;
	send_frame_udp_request(ipaddr, TRHEAD_RD, &mrd);
	
	return;
#endif
	
#ifdef COMM_CLIENT
	if(p_gd->data_len > IP_ADDR_MAX_SIZE)
	{
		return;
	}

	cli_info_t *m_info = calloc(1, sizeof(cli_info_t));

	switch(p_gd->tr_info)
	{
	case TRINFO_IP:
		memcpy(m_info->ipaddr, p_gd->data, p_gd->data_len);
		m_info->ip_len = p_gd->data_len;
		break;

	case TRINFO_HOLD:
	case TRINFO_REG:
		memcpy(m_info->ipaddr, ipaddr, strlen(ipaddr));
		m_info->ip_len = strlen(ipaddr);
		break;
	}
	
	memcpy(m_info->cidentify_no, p_gd->cidentify_no, sizeof(cidentify_no_t));	
	m_info->trans_type = p_gd->trans_type;
	GET_SERVER_IP(m_info->serverip_addr);
	m_info->serverip_len = strlen(m_info->serverip_addr);
	m_info->check_count = 0;
	m_info->check_conn = 1;
	m_info->user_info = NULL;
	m_info->next = NULL;
	
	if(add_client_info(m_info) != 0)
	{
		free(m_info);
	}

	set_cli_check(query_client_info(p_gd->cidentify_no));

	dev_info_t *p_dev = get_gateway_info()->p_dev;
	char buffer[ZDEVICE_MAX_NUM<<2] = {0};
	char bsize = 0;
	while(p_dev != NULL && bsize < (ZDEVICE_MAX_NUM<<2))
	{
		incode_xtoc16(buffer+bsize, p_dev->znet_addr);
		bsize += 4;
		p_dev = p_dev->next;
	}

	if(p_gd->tr_info != TRINFO_HOLD)
	{
		gd_t mgd;
		memcpy(mgd.zidentify_no, get_gateway_info()->gw_no, sizeof(zidentify_no_t));
		memcpy(mgd.cidentify_no, p_gd->cidentify_no, sizeof(cidentify_no_t));
		mgd.trans_type = TRTYPE_UDP_TRAVERSAL;
		mgd.tr_info = TRINFO_REG;
		mgd.data = buffer;
		mgd.data_len = bsize;
		send_frame_udp_request(
			query_client_info(p_gd->cidentify_no)->ipaddr, TRHEAD_GD, &mgd);
	}

	return;
#endif


#ifdef CLIENT_TEST
	rd_t mrd;
	memcpy(mrd.zidentify_no, p_gd->zidentify_no, sizeof(zidentify_no_t));
	memcpy(mrd.cidentify_no, p_gd->cidentify_no, sizeof(cidentify_no_t));
	mrd.trans_type = TRTYPE_UDP_NORMAL;
	mrd.tr_info = TRINFO_NONE;
	mrd.data = NULL;
	mrd.data_len = 0;
	send_frame_udp_request(ipaddr, TRHEAD_RD, &mrd);
#endif
}

void rd_handler(struct sockaddr_in *addr, rd_t *p_rd)
{
	char ipaddr[24] = {0};
	sprintf(ipaddr, "%s:%u", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
	
#ifdef COMM_CLIENT
	cli_info_t *p_cli = query_client_info(p_rd->cidentify_no);

	p_cli->trans_type = p_rd->trans_type;
	p_cli->ip_len = strlen(ipaddr);
	memcpy(p_cli->ipaddr, ipaddr, p_cli->ip_len);
	
	set_cli_check(p_cli);
#endif
}

void dc_handler(struct sockaddr_in *addr, dc_t *p_dc)
{
	char ipaddr[24] = {0};
	sprintf(ipaddr, "%s:%u", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
	
#ifdef COMM_CLIENT
	cli_info_t *m_info = calloc(1, sizeof(cli_info_t));
	memcpy(m_info->cidentify_no, p_dc->cidentify_no, sizeof(cidentify_no_t));	
	m_info->trans_type = p_dc->trans_type;
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
	m_info->user_info = NULL;
	m_info->next = NULL;
	
	if(add_client_info(m_info) != 0)
	{
		free(m_info);
	}

	fr_head_type_t head_type = get_frhead_from_str(p_dc->data);
	void *p = get_frame_alloc(head_type, p_dc->data, p_dc->data_len);
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
		if(memcmp(p_mgw->gw_no, p_dc->zidentify_no, sizeof(zidentify_no_t)))
		{
			dev_info = query_zdevice_info_with_sn(p_dc->zidentify_no);
		}
		else
		{
			set_devopt_fromstr(p_mgw->zgw_opt, de->data, de->data_len);

			if(p_mgw->zgw_opt && 
				(p_mgw->zgw_opt->type == FRAPP_DOOR_SENSOR
					|| p_mgw->zgw_opt->type == FRAPP_IR_DETECTION
					|| (p_mgw->zgw_opt->type == FRAPP_ENVDETECTION
						&& !memcmp(p_mgw->zgw_opt
									->device.envdetection.current_buffer, 
								DEVOPT_AIRCONTROLLER_UPSETTING, 3))
					|| (p_mgw->zgw_opt->type == FRAPP_AIRCONTROLLER
						&& !memcmp(p_mgw->zgw_opt
									->device.aircontroller.current_buffer, 
								DEVOPT_AIRCONTROLLER_UPSETTING, 3))))
			{
				ur_t ur;
				memcpy(ur.head, FR_HEAD_UR, 3);
				ur.type = get_frnet_type_to_ch(FRNET_ROUTER);
				get_frapp_type_to_str(ur.ed_type, p_mgw->zapp_type);
				incode_xtoc16(ur.short_addr, 0);
				ur.data_len = de->data_len;
				ur.data = de->data;
				memcpy(ur.tail, FR_TAIL, 4);
									
				fr_buffer_t *frbuffer = get_switch_buffer_alloc(HEAD_UR, 
										p_mgw->zgw_opt, &ur);
		
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
						enable_datalog_atime();
						send_frame_udp_request(p_cli->serverip_addr, TRHEAD_UB, &ub);
					}
					else if(p_cli->trans_type == TRTYPE_UDP_TRAVERSAL)
					{
						enable_datalog_atime();
						send_frame_udp_request(p_cli->ipaddr, TRHEAD_UB, &ub);
					}

					p_cli = p_cli->next;
				}
				get_buffer_free(frbuffer);
				
				goto Handle_UR_free;
			}
			else if(p_mgw->zgw_opt && p_mgw->zgw_opt->type == FRAPP_HUELIGHT)
			{
				memcpy(p_mgw->zgw_opt->device.huelight.sclient, 
							p_dc->cidentify_no, sizeof(cidentify_no_t));
			}
			
			buffer = get_switch_buffer_alloc(HEAD_DE, p_mgw->zgw_opt, de);
		}
		
		if(dev_info != NULL)
		{
			set_devopt_fromstr(dev_info->zdev_opt, de->data, de->data_len);

			if(dev_info->zdev_opt && 
				(dev_info->zdev_opt->type == FRAPP_DOOR_SENSOR
					|| dev_info->zdev_opt->type == FRAPP_IR_DETECTION
					|| (dev_info->zdev_opt->type == FRAPP_ENVDETECTION
						&& !memcmp(dev_info->zdev_opt
									->device.envdetection.current_buffer, 
								DEVOPT_AIRCONTROLLER_UPSETTING, 3))
					|| (dev_info->zdev_opt->type == FRAPP_AIRCONTROLLER
						&& !memcmp(dev_info->zdev_opt
									->device.aircontroller.current_buffer, 
								DEVOPT_AIRCONTROLLER_UPSETTING, 3))))
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
						enable_datalog_atime();
						send_frame_udp_request(p_cli->serverip_addr, TRHEAD_UB, &ub);
					}
					else if(p_cli->trans_type == TRTYPE_UDP_TRAVERSAL)
					{
						enable_datalog_atime();
						send_frame_udp_request(p_cli->ipaddr, TRHEAD_UB, &ub);
					}

					p_cli = p_cli->next;
				}
				get_buffer_free(frbuffer);
				
				goto Handle_UR_free;
			}
			else if(dev_info->zdev_opt && dev_info->zdev_opt->type == FRAPP_HUELIGHT)
			{
				memcpy(dev_info->zdev_opt->device.huelight.sclient, 
							p_dc->cidentify_no, sizeof(cidentify_no_t));
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

#ifdef REMOTE_UPDATE_APK
	if(!memcmp(p_dc->zidentify_no, p_dc->cidentify_no, sizeof(cidentify_no_t))
		&& p_dc->trans_type == TRTYPE_UDP_NORMAL
		&& p_dc->tr_info == TRINFO_UPDATE)
	{
		if(p_dc->data_len >= 2 && !memcmp(p_dc->data, "LS", 2))
		{
			reapk_version_code("LS", ipaddr, p_dc->cidentify_no);
		}
		else
		{
			reapk_version_code(NULL, ipaddr, p_dc->cidentify_no);
		}
		return;
	}
#endif
	
	while(p_gw != NULL)
	{
		if(!memcmp(p_gw->gw_no, p_dc->zidentify_no, sizeof(zidentify_no_t)))
		{
			goto gwdev_match;
		}
		
		dev_info_t *p_dev = p_gw->p_dev;
		while(p_dev != NULL)
		{
			if(!memcmp(p_dev->zidentity_no, p_dc->zidentify_no, sizeof(zidentify_no_t)))
			{
				goto gwdev_match;
			}

			p_dev = p_dev->next;
		}

		p_gw = p_gw->next;
	}

	ub_t ub;
	memcpy(ub.zidentify_no, p_dc->zidentify_no, sizeof(zidentify_no_t));
	memcpy(ub.cidentify_no, p_dc->cidentify_no, sizeof(cidentify_no_t));
	ub.trans_type = TRTYPE_UDP_NORMAL;
	ub.tr_info = TRINFO_DISMATCH;
	ub.data = NULL;
	ub.data_len = 0;
	enable_datalog_atime();
	send_frame_udp_request(ipaddr, TRHEAD_UB, &ub);
	
	return;

gwdev_match:
	memcpy(mdc.zidentify_no, p_dc->zidentify_no, sizeof(zidentify_no_t));
	memcpy(mdc.cidentify_no, p_dc->cidentify_no, sizeof(cidentify_no_t));
	mdc.trans_type = TRTYPE_UDP_NORMAL;
	mdc.tr_info = p_dc->tr_info;
	mdc.data = p_dc->data;
	mdc.data_len = p_dc->data_len;
	enable_datalog_atime();
	send_frame_udp_request(p_gw->ipaddr, TRHEAD_DC, &mdc);
	return;
#endif
}

void ub_handler(struct sockaddr_in *addr, ub_t *p_ub)
{
#ifdef COMM_SERVER
	ub_t mub;
	gw_info_t *p_gw = NULL;
	cli_contain_t *t_contain = NULL;

	if(!memcmp(p_ub->cidentify_no, get_common_no(), sizeof(cidentify_no_t))
		&& p_ub->trans_type == TRTYPE_UDP_TRANS)
	{
		p_gw = get_gateway_list()->p_gw;
		while(p_gw != NULL)
		{
			if(!memcmp(p_gw->gw_no, p_ub->zidentify_no, sizeof(zidentify_no_t)))
			{
				goto trans_cli_match;
			}
			
			dev_info_t *p_dev = p_gw->p_dev;
			while(p_dev != NULL)
			{
				if(!memcmp(p_dev->zidentity_no, p_ub->zidentify_no, sizeof(zidentify_no_t)))
				{
					goto trans_cli_match;
				}

				p_dev = p_dev->next;
			}

			p_gw = p_gw->next;
		}
		
		return;
	}
	
	cli_info_t *p_cli = get_client_list()->p_cli;
	while(p_cli != NULL)
	{
		if(!memcmp(p_cli->cidentify_no, p_ub->cidentify_no, sizeof(cidentify_no_t)))
		{
			goto client_match;
		}

		p_cli = p_cli->next;
	}

	return;

client_match:	
	memcpy(mub.zidentify_no, p_ub->zidentify_no, sizeof(zidentify_no_t));
	memcpy(mub.cidentify_no, p_ub->cidentify_no, sizeof(cidentify_no_t));
	mub.trans_type = TRTYPE_UDP_NORMAL;
	mub.tr_info = p_ub->tr_info;
	mub.data = p_ub->data;
	mub.data_len = p_ub->data_len;
	//enable_datalog_atime();
	send_frame_udp_request(p_cli->ipaddr, TRHEAD_UB, &mub);
	return;

trans_cli_match:
	memcpy(mub.zidentify_no, p_ub->zidentify_no, sizeof(zidentify_no_t));
	memcpy(mub.cidentify_no, p_ub->cidentify_no, sizeof(cidentify_no_t));
	mub.trans_type = TRTYPE_UDP_NORMAL;
	mub.tr_info = p_ub->tr_info;
	mub.data = p_ub->data;
	mub.data_len = p_ub->data_len;
	
	t_contain = p_gw->p_contain;
	while(t_contain != NULL)
	{
		memset(mub.cidentify_no, 0, sizeof(cidentify_no_t));
		memcpy(mub.cidentify_no, 
			t_contain->p_cli->cidentify_no, sizeof(cidentify_no_t));
		//enable_datalog_atime();
		send_frame_udp_request(t_contain->p_cli->ipaddr, TRHEAD_UB, &mub);
		t_contain = t_contain->next;
	}
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
	case TRHEAD_UL: 
	case TRHEAD_SL: 
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

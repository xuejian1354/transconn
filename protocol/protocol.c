/*
 * protocol.c
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
#include "protocol.h"
#include <mincode.h>
#include <module/serial.h>
#include <protocol/trframelysis.h>
#include <protocol/trrequest.h>
#include <services/mevent.h>

#ifdef COMM_CLIENT
static gw_info_t gw_info;
#endif

static cli_list_t cli_list;

#ifdef COMM_SERVER
static gw_list_t gw_list;
#endif

#ifdef COMM_CLIENT
gw_info_t *get_gateway_info()
{
	return &gw_info;
}
#endif

cli_list_t *get_client_list()
{
	return &cli_list;
}


#ifdef COMM_SERVER
gw_list_t *get_gateway_list()
{
	return &gw_list;
}
#endif

uint8 *get_zdev_buffer_alloc(dev_info_t *dev_info)
{
	uint8 *dev_buffer = (uint8 *)calloc(1, ZDEVICE_BUFFER_SIZE);
	incode_xtocs(dev_buffer, dev_info->zidentity_no, 8);
	incode_xtoc16(dev_buffer+16, dev_info->znet_addr);
	get_frapp_type_to_str(dev_buffer+20, dev_info->zapp_type);
	dev_buffer[22] = get_frnet_type_to_ch(dev_info->znet_type);

	return dev_buffer;
}


void get_zdev_buffer_free(uint8 *p)
{
	free(p);
}

dev_info_t *get_zdev_frame_alloc(uint8 *buffer, int length)
{
	if(length < ZDEVICE_BUFFER_SIZE || length > TR_BUFFER_SIZE)
	{
		return NULL;
	}

	dev_info_t *dev_info = (dev_info_t *)calloc(1, sizeof(dev_info_t));
	incode_ctoxs(dev_info->zidentity_no, buffer, 16);
	incode_ctox16(&dev_info->znet_addr, buffer+16);
	dev_info->zapp_type = get_frapp_type_from_str(buffer+20);
	dev_info->znet_type = get_frnet_type_from_str(buffer[22]);
	dev_info->zdev_opt = NULL;
	dev_info->next = NULL;

	return dev_info;
}


void get_zdev_frame_free(dev_info_t *p)
{	
	free(p);
}


uint8 *get_gateway_buffer_alloc(gw_info_t *gw_info)
{
	if(gw_info->ip_len > IP_ADDR_MAX_SIZE)
	{
		return NULL;
	}
	
	uint8 *gw_buffer = (uint8 *)calloc(1, GATEWAY_BUFFER_FIX_SIZE+gw_info->ip_len);
	incode_xtocs(gw_buffer, gw_info->gw_no, 8);
	incode_xtoc16(gw_buffer+16, gw_info->zpanid);
	incode_xtoc16(gw_buffer+20, gw_info->zchannel);
	incode_xtoc32(gw_buffer+24, gw_info->rand);
	memcpy(gw_buffer+32, gw_info->ipaddr, gw_info->ip_len);

	return gw_buffer;
}


void get_gateway_buffer_free(uint8 *p)
{
	free(p);
}

gw_info_t *get_gateway_frame_alloc(uint8 *buffer, int length)
{
	if(length < GATEWAY_BUFFER_FIX_SIZE || length > TR_BUFFER_SIZE)
	{
		return NULL;
	}

	gw_info_t *gw_info = (gw_info_t *)calloc(1, sizeof(gw_info_t));
	incode_ctoxs(gw_info->gw_no, buffer, 16);
	incode_ctox16(&gw_info->zpanid, buffer+16);
	incode_ctox16(&gw_info->zchannel, buffer+20);
	incode_ctox32(&gw_info->rand, buffer+24);
	gw_info->ip_len = length-GATEWAY_BUFFER_FIX_SIZE;
	memset(gw_info->ipaddr, 0, sizeof(gw_info->ipaddr));
	memcpy(gw_info->ipaddr, buffer+32, gw_info->ip_len);
	if(pthread_mutex_init(&gw_info->lock, NULL) != 0)
    {
		free(gw_info);
        return NULL;
    }
	
	gw_info->p_dev = NULL;
	gw_info->next = NULL;

	return gw_info;
}


void get_gateway_frame_free(gw_info_t *p)
{
	if(p->p_dev != NULL)
	{
		dev_info_t *pre_dev = p->p_dev;
		dev_info_t *pdev = p->p_dev;

		while(pdev != NULL)
		{
			pre_dev = pdev;
			pdev = pdev->next;
			free(pre_dev);
		}
	}
	
	free(p);
}


int add_zdev_info(gw_info_t *gw_info, dev_info_t *m_dev)
{
	dev_info_t *pre_dev =  NULL;
	dev_info_t *t_dev = gw_info->p_dev;

	if(m_dev == NULL)
	{
		return -1;
	}
	else
	{
		m_dev->next = NULL;
	}

	while(t_dev != NULL)
	{
		if(t_dev->znet_addr != m_dev->znet_addr)
		{
			pre_dev = t_dev;
			t_dev = t_dev->next;
		}
		else
		{
			if(memcmp(t_dev->zidentity_no, m_dev->zidentity_no, 8)
				|| t_dev->zapp_type != m_dev->zapp_type
				|| t_dev->znet_type != m_dev->znet_type)
			{
				memcpy(t_dev->zidentity_no, m_dev->zidentity_no, 8);
				t_dev->zapp_type = m_dev->zapp_type;
				t_dev->znet_type = m_dev->znet_type;
			}

			set_devopt_data_fromopt(t_dev->zdev_opt, m_dev->zdev_opt);

			if(pre_dev != NULL)
			{
				pthread_mutex_lock(&gw_info->lock);
				pre_dev->next = t_dev->next;
				t_dev->next = gw_info->p_dev;
				gw_info->p_dev = t_dev;
				pthread_mutex_unlock(&gw_info->lock);
			}
			
			return 1;
		}
	}

	pthread_mutex_lock(&gw_info->lock);
	m_dev->next = gw_info->p_dev;
	gw_info->p_dev = m_dev;
	pthread_mutex_unlock(&gw_info->lock);

	return 0;
}



dev_info_t *query_zdev_info(gw_info_t *gw_info, uint16 znet_addr)
{
	dev_info_t *t_dev = gw_info->p_dev;


	while(t_dev != NULL)
	{
		if(t_dev->znet_addr != znet_addr)
		{
			t_dev = t_dev->next;
		}
		else
		{
			return t_dev;
		}
	}

	return NULL;
}

int del_zdev_info(gw_info_t *gw_info, uint16 znet_addr)
{
	dev_info_t *pre_dev =  NULL;
	dev_info_t *t_dev = gw_info->p_dev;


	while(t_dev != NULL)
	{
		if(t_dev->znet_addr != znet_addr)
		{
			pre_dev = t_dev;
			t_dev = t_dev->next;
		}
		else
		{
			pthread_mutex_lock(&gw_info->lock);
			if(pre_dev != NULL)
			{
				pre_dev->next = t_dev->next;
			}
			else
			{
				gw_info->p_dev = t_dev->next;
			}
			pthread_mutex_unlock(&gw_info->lock);

			get_devopt_data_free(t_dev->zdev_opt);
			free(t_dev);
			return 0;
		}
	}

	return -1;
}


frhandler_arg_t *get_frhandler_arg_alloc(int fd, struct sockaddr_in *addr, 
														char *buf, int len)
{
	if(len > MAXSIZE)
	{
		return NULL;
	}

	frhandler_arg_t *arg = calloc(1, sizeof(frhandler_arg_t));
	arg->buf = calloc(1, len);

	arg->fd = fd;

	if(addr != NULL)
	{
		memcpy(&arg->addr, addr, sizeof(struct sockaddr_in));
	}
	
	if(buf != NULL)
	{
		memcpy(arg->buf, buf, len);
		arg->len = len;
	}
	else
	{
		free(arg->buf);
		arg->buf = NULL;
		arg->len = 0;
	}

	return arg;
}

void get_frhandler_arg_free(frhandler_arg_t *arg)
{
	if(arg != NULL)
	{
		free(arg->buf);
		free(arg);
	}
}

int add_client_info(cli_info_t *m_info)
{
	cli_info_t *pre_before, *pre_cli =  NULL;
	cli_info_t *t_cli = cli_list.p_cli;

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
			memset(t_cli->ipaddr, 0, sizeof(t_cli->ipaddr));
			memcpy(t_cli->ipaddr, m_info->ipaddr, m_info->ip_len);
			memset(t_cli->serverip_addr, 0, sizeof(t_cli->serverip_addr));
			memcpy(t_cli->serverip_addr, m_info->serverip_addr, m_info->serverip_len);
			t_cli->check_count = m_info->check_count;
			t_cli->check_conn = m_info->check_conn;

			if(pre_cli != NULL)
			{
				pthread_mutex_lock(&cli_list.lock);
				pre_cli->next = t_cli->next;
				t_cli->next = cli_list.p_cli;
				cli_list.p_cli = t_cli;
				pthread_mutex_unlock(&cli_list.lock);
			}
			
			return 1;
		}
	}

	pthread_mutex_lock(&cli_list.lock);
	m_info->next = cli_list.p_cli;
	cli_list.p_cli = m_info;
#ifdef COMM_SERVER
	if(cli_list.max_num >= SERVER_CLI_LIST_MAX_NUM)
#else
	if(cli_list.max_num >= GATEWAY_CLI_LIST_MAX_NUM)
#endif
	{
		if(pre_before != NULL)
			pre_before->next = NULL;
		
		free(pre_cli);
	}
	else
	{
		cli_list.max_num++;
	}
	pthread_mutex_unlock(&cli_list.lock);

	return 0;
}

cli_info_t *query_client_info(cidentify_no_t cidentify_no)
{
	cli_info_t *t_cli = cli_list.p_cli;

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
	cli_info_t *t_cli = cli_list.p_cli;

	while(t_cli != NULL)
	{
		if(memcmp(t_cli->cidentify_no, cidentify_no, sizeof(cidentify_no_t)))
		{
			pre_cli = t_cli;
			t_cli = t_cli->next;
		}
		else
		{
			pthread_mutex_lock(&cli_list.lock);
			if(pre_cli != NULL)
			{
				pre_cli->next = t_cli->next;
			}
			else
			{
				cli_list.p_cli = t_cli->next;
			}
			
			cli_list.max_num--;
			pthread_mutex_unlock(&cli_list.lock);

			free(t_cli);
			return 0;
		}
	}

	return -1;
}

#ifdef COMM_CLIENT
int add_zdevice_info(dev_info_t *m_dev)
{
	return add_zdev_info(&gw_info, m_dev);
}

dev_info_t *query_zdevice_info(uint16 znet_addr)
{
	return query_zdev_info(&gw_info, znet_addr);
}

dev_info_t *query_zdevice_info_with_sn(zidentify_no_t zidentify_no)
{
	dev_info_t *t_dev = gw_info.p_dev;


	while(t_dev != NULL)
	{
		if(memcmp(t_dev->zidentity_no, zidentify_no, sizeof(zidentify_no_t)))
		{
			t_dev = t_dev->next;
		}
		else
		{
			return t_dev;
		}
	}

	return NULL;
}

int del_zdevice_info(uint16 znet_addr)
{
	return del_zdev_info(&gw_info, znet_addr);
}

#elif defined(COMM_SERVER)
int add_gateway_info(gw_info_t *m_gw)
{
	gw_info_t *pre_before, *pre_gw =  NULL;
	gw_info_t *t_gw = gw_list.p_gw;

	if(m_gw == NULL)
	{
		return -1;
	}
	else
	{
		m_gw->next = NULL;
	}

	while(t_gw != NULL)
	{
		if(memcmp(t_gw->gw_no, m_gw->gw_no, sizeof(zidentify_no_t)))
		{
			pre_before = pre_gw;
			pre_gw = t_gw;
			t_gw = t_gw->next;
		}
		else
		{
			t_gw->zpanid = m_gw->zpanid;
			t_gw->zchannel = m_gw->zchannel;
			t_gw->rand = m_gw->rand;
			t_gw->ip_len = m_gw->ip_len;
			memset(t_gw->ipaddr, 0, sizeof(t_gw->ipaddr));
			memcpy(t_gw->ipaddr, m_gw->ipaddr, m_gw->ip_len);
			memset(t_gw->serverip_addr, 0, sizeof(t_gw->serverip_addr));
			memcpy(t_gw->serverip_addr, m_gw->serverip_addr, m_gw->serverip_len);

			if(pre_gw != NULL)
			{
				pthread_mutex_lock(&gw_list.lock);
				pre_gw->next = t_gw->next;
				t_gw->next = gw_list.p_gw;
				gw_list.p_gw = t_gw;
				pthread_mutex_unlock(&gw_list.lock);
			}
			
			return 1;
		}
	}

	pthread_mutex_lock(&gw_list.lock);
	m_gw->next = gw_list.p_gw;
	gw_list.p_gw = m_gw;
	if(gw_list.max_num >= SERVER_GW_LIST_MAX_NUM)
	{
		if(pre_before != NULL)
			pre_before->next = NULL;
		
		free(pre_gw);
	}
	else
	{
		gw_list.max_num++;
	}
	pthread_mutex_unlock(&gw_list.lock);

	return 0;
}

gw_info_t *query_gateway_info(zidentify_no_t gw_no)
{
	gw_info_t *t_gw = gw_list.p_gw;


	while(t_gw != NULL)
	{
		if(memcmp(t_gw->gw_no, gw_no, sizeof(zidentify_no_t)))
		{
			t_gw = t_gw->next;
		}
		else
		{
			return t_gw;
		}
	}

	return NULL;
}

int del_gateway_info(zidentify_no_t gw_no)
{
	gw_info_t *pre_gw =  NULL;
	gw_info_t *t_gw = gw_list.p_gw;


	while(t_gw != NULL)
	{
		if(memcmp(t_gw->gw_no, gw_no, sizeof(zidentify_no_t)))
		{
			pre_gw = t_gw;
			t_gw = t_gw->next;
		}
		else
		{	
			pthread_mutex_lock(&gw_list.lock);
			if(pre_gw != NULL)
			{
				pre_gw->next = t_gw->next;
			}
			else
			{
				gw_list.p_gw = t_gw->next;
			}
			
			gw_list.max_num--;
			pthread_mutex_unlock(&gw_list.lock);

			dev_info_t *pre_dev = t_gw->p_dev;
			dev_info_t *pdev = t_gw->p_dev;
			
			while(pdev != NULL)
			{
				pre_dev = pdev;
				pdev = pdev->next;
				free(pre_dev);
			}

			free(t_gw);
			return 0;
		}
	}

	return -1;
}
#endif


#ifdef COMM_CLIENT
void analysis_zdev_frame(frhandler_arg_t *arg)
{
	if(arg == NULL)
	{
		return;
	}
	
	dev_info_t *dev_info;
	uint8 *buffer;
	uint16 znet_addr;

	char ipaddr[24] = {0};
	GET_SERVER_IP(ipaddr);
	
	fr_head_type_t head_type = get_frhead_from_str(arg->buf);
	
	void *p = get_frame_alloc(head_type, arg->buf, arg->len);

	if(p == NULL)
	{
		return;
	}

	switch(head_type)
	{
	case HEAD_UC:
	{
		uc_t *uc = (uc_t *)p;
		incode_ctoxs(gw_info.gw_no, uc->ext_addr, 16);
		incode_ctox16(&gw_info.zpanid, uc->panid);
		incode_ctox16(&gw_info.zchannel, uc->channel);
		gw_info.rand = gen_rand(gw_info.gw_no);
		
		buffer = get_gateway_buffer_alloc(&gw_info);
		
		bi_t bi;
		memcpy(bi.sn, get_gateway_info()->gw_no, sizeof(zidentify_no_t));
		bi.trans_type = TRTYPE_UDP_NORMAL;
		bi.fr_type = TRFRAME_PUT_GW;
		bi.data = buffer;
		bi.data_len = GATEWAY_BUFFER_FIX_SIZE+gw_info.ip_len;
		send_frame_udp_request(ipaddr, TRHEAD_BI, &bi);
		
		get_gateway_buffer_free(buffer);
		get_frame_free(HEAD_UC, uc);
	}
	break;
		
	case HEAD_UO:
	{
		uo_t *uo = (uo_t *)p;
		dev_info = calloc(1, sizeof(dev_info_t));
		incode_ctoxs(dev_info->zidentity_no, uo->ext_addr, 16);
		incode_ctox16(&dev_info->znet_addr, uo->short_addr);
		dev_info->zapp_type = get_frapp_type_from_str(uo->ed_type);
		dev_info->znet_type = get_frnet_type_from_str(uo->type);

		dev_info->zdev_opt = get_devopt_data_alloc(dev_info->zapp_type, 
													uo->data, uo->data_len);
		
#ifdef DE_ZDEVICE_RECORD
		FILE *fp = NULL;
		if((fp = fopen(RECORD_FILE, "a+")) != NULL)
		{
			char sn[20] = {0};
			incode_xtocs(sn, dev_info->zidentity_no, 8);
			fprintf(fp, "[zdevice]\nS/N: %s\nShort Addr: %04X\nDev Type: %02d\n\n", 
				sn, dev_info->znet_addr, dev_info->zapp_type);
			fclose(fp);
		}
#endif
		set_zdev_check(dev_info->znet_addr);


		fr_buffer_t *frbuffer = get_switch_buffer_alloc(HEAD_UO, 
											dev_info->zdev_opt, uo);
		
		cli_info_t *p_cli = get_client_list()->p_cli;
		while(p_cli != NULL)
		{
			ub_t ub;
			memcpy(ub.zidentify_no, get_gateway_info()->gw_no, sizeof(zidentify_no_t));
			memcpy(ub.cidentify_no, p_cli->cidentify_no, sizeof(cidentify_no_t));
			ub.trans_type = TRTYPE_UDP_NORMAL;
			ub.tr_info = TRINFO_REDATA;
			ub.data = frbuffer->data;
			ub.data_len = frbuffer->size;
			send_frame_udp_request(p_cli->ipaddr, TRHEAD_UB, &ub);

			p_cli = p_cli->next;
		}
		get_buffer_free(frbuffer);
		
		if(add_zdevice_info(dev_info) != 0)
		{
			get_devopt_data_free(dev_info->zdev_opt);
			free(dev_info);
		}
		else
		{
			buffer = get_zdev_buffer_alloc(dev_info);

			bi_t bi;
			memcpy(bi.sn, get_gateway_info()->gw_no, sizeof(zidentify_no_t));
			bi.trans_type = TRTYPE_UDP_NORMAL;
			bi.fr_type = TRFRAME_PUT_DEV;
			bi.data = buffer;
			bi.data_len = ZDEVICE_BUFFER_SIZE;
			send_frame_udp_request(ipaddr, TRHEAD_BI, &bi);
			
			get_zdev_buffer_free(buffer);
		}

		get_frame_free(HEAD_UO, uo);
	}
	break;
		
	case HEAD_UH:
	{
		uh_t *uh = (uh_t *)p;
		incode_ctox16(&znet_addr, uh->short_addr);
		dev_info = query_zdevice_info(znet_addr);
		if(dev_info == NULL)
		{
			uint8 mbuf[16] = {0};
			sprintf(mbuf, "D:/SR/%04X:O\r\n", znet_addr);
			serial_write(mbuf, 14);
		}
		else
		{
			set_zdev_check(znet_addr);
		}
		get_frame_free(HEAD_UH, uh);
	}
	break;
		
	case HEAD_UR:
	{
		ur_t *ur = (ur_t *)p;
		incode_ctox16(&znet_addr, ur->short_addr);
		dev_info = query_zdevice_info(znet_addr);
		if(dev_info == NULL)
		{
			uint8 mbuf[16] = {0};
			sprintf(mbuf, "D:/SR/%04X:O\r\n", znet_addr);
			serial_write(mbuf, 14);
		}
		else
		{
			dev_opt_t *opt = get_devopt_data_alloc(dev_info->zapp_type, 
													ur->data, ur->data_len);
			set_devopt_data_fromopt(dev_info->zdev_opt, opt);
			get_devopt_data_free(opt);

			//devopt_de_print(dev_info->zdev_opt);

			if(dev_info->zdev_opt->type == FRAPP_DOOR_SENSOR
				&& !dev_info->zdev_opt->device.doorsensor.setting)
			{
				goto UR_FREE;
			}

			if(dev_info->zdev_opt->type == FRAPP_IR_DETECTION
				&& !dev_info->zdev_opt->device.irdetect.setting)
			{
				goto UR_FREE;
			}

			fr_buffer_t *frbuffer = get_switch_buffer_alloc(HEAD_UR, 
											dev_info->zdev_opt, ur);
			
			cli_info_t *p_cli = get_client_list()->p_cli;
			while(p_cli != NULL)
			{
				ub_t ub;
				memcpy(ub.zidentify_no, get_gateway_info()->gw_no, sizeof(zidentify_no_t));
				memcpy(ub.cidentify_no, p_cli->cidentify_no, sizeof(cidentify_no_t));
				ub.trans_type = TRTYPE_UDP_NORMAL;
				ub.tr_info = TRINFO_REDATA;
				ub.data = frbuffer->data;
				ub.data_len = frbuffer->size;
				send_frame_udp_request(p_cli->ipaddr, TRHEAD_UB, &ub);

				p_cli = p_cli->next;
			}
			get_buffer_free(frbuffer);
		}
UR_FREE:
		get_frame_free(HEAD_UR, ur);
	}
	break;
		
	case HEAD_DE:
	{
		de_t *de = (de_t *)p;
		get_frame_free(HEAD_DE, de);
	}
	break;
		
	default: break;
	}

	get_frhandler_arg_free(arg);
}
#endif

void analysis_capps_frame(frhandler_arg_t *arg)
{
	if(arg == NULL)
	{
		return;
	}
	
  	cli_info_t *cli_info;
	
	tr_head_type_t head_type = get_trhead_from_str(arg->buf);
	void *p = get_trframe_alloc(head_type, arg->buf, arg->len);
	
	if(p == NULL)
	{
		return;
	}
	
	switch(head_type)
	{
	case TRHEAD_PI:
	{
		pi_t *pi = (pi_t *)p;
		pi_handler(&arg->addr, pi);
		get_trframe_free(TRHEAD_PI, p);
	}
	break;
		
	case TRHEAD_BI:
	{
		bi_t *bi = (bi_t *)p;
		bi_handler(&arg->addr, bi);
		get_trframe_free(TRHEAD_BI, p);
	}
	break;
		
	case TRHEAD_GP:
	{
		gp_t *gp = (gp_t *)p;
		gp_handler(&arg->addr, gp);
		get_trframe_free(TRHEAD_GP, p);
	}
	break;
		
	case TRHEAD_RP:
	{
		rp_t *rp = (rp_t *)p;
		rp_handler(&arg->addr, rp);
		get_trframe_free(TRHEAD_RP, p);
	}
	break;
		
	case TRHEAD_GD:
	{
		gd_t *gd = (gd_t *)p;
		gd_handler(&arg->addr, gd);
		get_trframe_free(TRHEAD_GD, p);
	}
	break;
		
	case TRHEAD_RD:
	{
		rd_t *rd = (rd_t *)p;
		rd_handler(&arg->addr, rd);		
		get_trframe_free(TRHEAD_RD, p);
	}
	break;
		
	case TRHEAD_DC:
	{
		dc_t *dc = (dc_t *)p;
		dc_handler(&arg->addr, dc);
		get_trframe_free(TRHEAD_DC, p);
	}
	break;
		
	case TRHEAD_UB:
	{
		ub_t *ub = (ub_t *)p;
		ub_handler(&arg->addr, ub);
		get_trframe_free(TRHEAD_UB, p);
	}
	break;

	default: break;
	}

	get_frhandler_arg_free(arg);
}

fr_buffer_t *get_switch_buffer_alloc(fr_head_type_t head_type, 
	dev_opt_t *opt, void *frame)
{
	uint8 *bdata;
	uint8 blen;
	
	switch(head_type)
	{
	case HEAD_UO:
	{
		ur_t *p_uo = (ur_t *)frame;
		bdata = p_uo->data;
		blen = p_uo->data_len;

		fr_buffer_t *data_buffer = get_devopt_buffer_alloc(opt);
		p_uo->data = data_buffer->data;
		p_uo->data_len = data_buffer->size;
		
		fr_buffer_t *frbuffer = get_buffer_alloc(HEAD_UO, p_uo);

		p_uo->data = bdata;
		p_uo->data_len = blen;
		get_devopt_buffer_free(data_buffer);
		
		return frbuffer;
	}
	break;

	case HEAD_UR:
	{
		ur_t *p_ur = (ur_t *)frame;
		bdata = p_ur->data;
		blen = p_ur->data_len;

		fr_buffer_t *data_buffer = get_devopt_buffer_alloc(opt);
		p_ur->data = data_buffer->data;
		p_ur->data_len = data_buffer->size;
		
		fr_buffer_t *frbuffer = get_buffer_alloc(HEAD_UR, p_ur);

		p_ur->data = bdata;
		p_ur->data_len = blen;
		get_devopt_buffer_free(data_buffer);
		
		return frbuffer;
	}

	case HEAD_DE:
	{
		de_t *p_de = (de_t *)frame;
		bdata = p_de->data;
		blen = p_de->data_len;

		fr_buffer_t *data_buffer = get_devopt_data_to_str(opt);
		p_de->data = data_buffer->data;
		p_de->data_len = data_buffer->size;

		fr_buffer_t *buffer = get_buffer_alloc(HEAD_DE, p_de);
		
		p_de->data = bdata;
		p_de->data_len = blen;
		get_buffer_free(data_buffer);

		return buffer;
	}

	default: return NULL;
	}
}

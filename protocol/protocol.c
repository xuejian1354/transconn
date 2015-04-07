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

int add_zdevice_info(dev_info_t *m_dev);
dev_info_t *query_zdevice_info(uint16 znet_addr);
int del_zdevice_info(uint16 znet_addr);


static gw_info_t gw_info = 
{
	{0},
	0,
	0,
	NULL,
};

static cli_list_t cli_list = 
{
	NULL,
};


#ifdef COMM_CLIENT
void analysis_zdev_frame(char *buf, int len)
{
	uc_t *uc;
	uo_t *uo;
	uh_t *uh;
	ur_t *ur;
	de_t *de;
	dev_info_t *dev_info;
	uint16 znet_addr;
	
	fr_head_type_t head_type = get_frhead_from_str(buf);
	
	void *p = get_frame_alloc(head_type, buf, len);

	if(p == NULL)
	{
		return;
	}

	switch(head_type)
	{
	case HEAD_UC:
		uc = (uc_t *)p;
		incode_ctoxs(gw_info.zidentity_no, uc->ext_addr, 16);
		incode_ctox16(&gw_info.zpanid, uc->panid);
		incode_ctox16(&gw_info.zchannel, uc->channel);
		get_frame_free(HEAD_UC, uc);
		break;
		
	case HEAD_UO:
		uo = (uo_t *)p;
		dev_info = calloc(1, sizeof(dev_info_t));
		incode_ctoxs(dev_info->zidentity_no, uo->ext_addr, 16);
		incode_ctox16(&dev_info->znet_addr, uo->short_addr);
		dev_info->zapp_type = get_frapp_type_from_str(uo->ed_type);
		dev_info->znet_type = get_frnet_type_from_str(&uo->type);
		
		if(add_zdevice_info(dev_info) != 0)
		{
			free(dev_info);
		}
		get_frame_free(HEAD_UO, uo);
		break;
		
	case HEAD_UH:
		uh = (uh_t *)p;
		incode_ctox16(&znet_addr, uh->short_addr);
		dev_info = query_zdevice_info(znet_addr);
		if(dev_info == NULL)
		{
			uint8 mbuf[16] = {0};
			sprintf(mbuf, "D:/SR/%0X:O\r\n", znet_addr);
			serial_write(mbuf, 14);
		}
		get_frame_free(HEAD_UH, uh);
		break;
		
	case HEAD_UR:
		ur = (ur_t *)p;
		incode_ctox16(&znet_addr, ur->short_addr);
		dev_info = query_zdevice_info(znet_addr);
		if(dev_info == NULL)
		{
			uint8 mbuf[16] = {0};
			sprintf(mbuf, "D:/SR/%0X:O\r\n", znet_addr);
			serial_write(mbuf, 14);
		}
		get_frame_free(HEAD_UR, ur);
		break;
		
	case HEAD_DE:
		de = (de_t *)p;
		get_frame_free(HEAD_DE, de);
		break;
		
	default:
		break;
	}
}


void analysis_capps_frame(struct sockaddr_in addr, char *buf, int len)
{
	pi_t *pi;
	bi_t *bi;
	gp_t *gp;
	rp_t *rp;
	gd_t *gd;
	rd_t *rd;
	dc_t *dc;
	ub_t *ub;
	cli_info_t *cli_info;
	tr_head_type_t head_type = get_trhead_from_str(buf);
	void *p = get_trframe_alloc(head_type, buf, len);

	if(p == NULL)
	{
		return;
	}

	switch(head_type)
	{
	case TRHEAD_PI:
		pi = (pi_t *)p;
		break;
		
	case TRHEAD_BI:
		bi = (bi_t *)p;
		break;
		
	case TRHEAD_GP:
		gp = (gp_t *)p;
		break;
		
	case TRHEAD_RP:
		rp = (rp_t *)p;
		break;
		
	case TRHEAD_GD:
		gd = (gd_t *)p;
		break;
		
	case TRHEAD_RD:
		rd = (rd_t *)p;
		break;
		
	case TRHEAD_DC:
		dc = (dc_t *)p;
		break;
		
	case TRHEAD_UB:
		ub = (ub_t *)p;
		break;

	default:
		break;
	}
}
#endif

int add_zdevice_info(dev_info_t *m_dev)
{
	dev_info_t *p_dev = gw_info.p_dev;
	dev_info_t *pre_dev =  NULL;
	dev_info_t *t_dev = p_dev;

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

			if(pre_dev != NULL)
			{
				pre_dev->next = t_dev->next;
				t_dev->next = p_dev;
				p_dev = t_dev;
			}
			
			return 1;
		}
	}

	m_dev->next = p_dev;
	p_dev = m_dev;

	return 0;
}



dev_info_t *query_zdevice_info(uint16 znet_addr)
{
	dev_info_t *p_dev = gw_info.p_dev;
	dev_info_t *t_dev = p_dev;


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

int del_zdevice_info(uint16 znet_addr)
{
	dev_info_t *p_dev = gw_info.p_dev;
	dev_info_t *pre_dev =  NULL;
	dev_info_t *t_dev = p_dev;


	while(t_dev != NULL)
	{
		if(t_dev->znet_addr != znet_addr)
		{
			pre_dev = t_dev;
			t_dev = t_dev->next;
		}
		else
		{
			if(pre_dev != NULL)
			{
				pre_dev->next = t_dev->next;
			}
			else
			{
				p_dev = t_dev->next;
			}

			free(t_dev);
			return 0;
		}
	}

	return -1;
}
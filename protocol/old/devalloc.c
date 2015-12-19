/*
 * devalloc.c
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

#include "devalloc.h"

#ifdef __cplusplus
extern "C" {
#endif

dev_info_t *get_zdev_frame_alloc(uint8 *buffer, int length)
{
	if(length < ZDEVICE_BUFFER_SIZE)
	{
		return NULL;
	}

	dev_info_t *dev_info = (dev_info_t *)calloc(1, sizeof(dev_info_t));
	incode_ctoxs(dev_info->zidentity_no, (char *)buffer, 16);
	incode_ctox16(&dev_info->znet_addr, (char *)(buffer+16));
	dev_info->zapp_type = get_frapp_type_from_str((char *)(buffer+20));
	dev_info->znet_type = get_frnet_type_from_str((char)(buffer[22]));
	dev_info->zdev_opt = NULL;
	dev_info->isdata_change = 1;
	dev_info->next = NULL;

	return dev_info;
}

void get_zdev_frame_free(dev_info_t *p)
{	
	if(p != NULL)
	{
		get_devopt_free(p->zdev_opt);
	}
	
	free(p);
}

gw_info_t *get_gateway_frame_alloc(uint8 *buffer, int length)
{	
	if(length < GATEWAY_BUFFER_FIX_SIZE)
	{
		return NULL;
	}

	uint8 optdata_len;
	incode_ctoxs(&optdata_len, (char *)(buffer+length-2), 2);

	gw_info_t *gw_info = (gw_info_t *)calloc(1, sizeof(gw_info_t));
	incode_ctoxs(gw_info->gw_no, (char *)buffer, 16);
	gw_info->zapp_type = get_frapp_type_from_str((char *)(buffer+16));
	incode_ctox16(&gw_info->zpanid, (char *)(buffer+18));
	incode_ctox16(&gw_info->zchannel, (char *)(buffer+22));
	incode_ctox32(&gw_info->rand, (char *)(buffer+26));
	gw_info->ip_len = length-GATEWAY_BUFFER_FIX_SIZE-optdata_len-2;
	if(gw_info->ip_len < IP_ADDR_MAX_SIZE)
	{
		memset(gw_info->ipaddr, 0, sizeof(gw_info->ipaddr));
		memcpy(gw_info->ipaddr, buffer+34, gw_info->ip_len);
	}
	gw_info->zgw_opt = get_devopt_fromstr(gw_info->zapp_type, 
		buffer+34+gw_info->ip_len, optdata_len);
	
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
	if(p == NULL)
	{
		return;
	}

	free(p->zgw_opt);
	
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

fr_buffer_t *get_switch_buffer_alloc(fr_head_type_t head_type, 
	dev_opt_t *opt, void *frame)
{
	uint8 *bdata;
	uint8 blen;
	
	switch(head_type)
	{
	case HEAD_UC:
	{
		uc_t *p_uc = (uc_t *)frame;
		bdata = p_uc->data;
		blen = p_uc->data_len;

		fr_buffer_t *data_buffer = get_devopt_buffer_alloc(opt, bdata, blen);
		if(data_buffer != NULL)
		{
			p_uc->data = data_buffer->data;
			p_uc->data_len = data_buffer->size;
		}
		else
		{
			p_uc->data = NULL;
			p_uc->data_len = 0;
		}
		
		fr_buffer_t *frbuffer = get_buffer_alloc(HEAD_UC, p_uc);

		p_uc->data = bdata;
		p_uc->data_len = blen;
		get_devopt_buffer_free(data_buffer);
		
		return frbuffer;
	}

	case HEAD_UO:
	{
		uo_t *p_uo = (uo_t *)frame;
		bdata = p_uo->data;
		blen = p_uo->data_len;

		fr_buffer_t *data_buffer = get_devopt_buffer_alloc(opt, bdata, blen);
		if(data_buffer != NULL)
		{
			p_uo->data = data_buffer->data;
			p_uo->data_len = data_buffer->size;
		}
		else
		{
			p_uo->data = NULL;
			p_uo->data_len = 0;
		}
		
		fr_buffer_t *frbuffer = get_buffer_alloc(HEAD_UO, p_uo);

		p_uo->data = bdata;
		p_uo->data_len = blen;
		get_devopt_buffer_free(data_buffer);
		
		return frbuffer;
	}

	case HEAD_UR:
	{
		ur_t *p_ur = (ur_t *)frame;
		bdata = p_ur->data;
		blen = p_ur->data_len;

		fr_buffer_t *data_buffer = get_devopt_buffer_alloc(opt, bdata, blen);
		if(data_buffer != NULL)
		{
			p_ur->data = data_buffer->data;
			p_ur->data_len = data_buffer->size;
		}
		else
		{
			p_ur->data = NULL;
			p_ur->data_len = 0;
		}
		
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

		fr_buffer_t *data_buffer = get_devopt_data_tostr(opt);
		if(data_buffer != NULL)
		{
			p_de->data = data_buffer->data;
			p_de->data_len = data_buffer->size;
		}
		else
		{
			p_de->data = NULL;
			p_de->data_len = 0;
		}

		fr_buffer_t *buffer = get_buffer_alloc(HEAD_DE, p_de);
		
		p_de->data = bdata;
		p_de->data_len = blen;
		get_buffer_free(data_buffer);

		return buffer;
	}

	default: return NULL;
	}
}

#ifdef __cplusplus
}
#endif

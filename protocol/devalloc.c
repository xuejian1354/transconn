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

fr_buffer_t *get_gateway_buffer_alloc(gw_info_t *gw_info)
{
	uint8 *data;
	uint8 datalen;
	if(gw_info->ip_len > IP_ADDR_MAX_SIZE)
	{
		return NULL;
	}
	
	fr_buffer_t *frbuffer = get_devopt_data_to_str(gw_info->zgw_opt);
	if(frbuffer != NULL)
	{
		data = frbuffer->data;
		datalen = frbuffer->size;
	}
	else
	{
		data = NULL;
		datalen  = 0;
	}
	
	fr_buffer_t *gw_buffer =(fr_buffer_t *)calloc(1, sizeof(fr_buffer_t));
	gw_buffer->size = GATEWAY_BUFFER_FIX_SIZE + gw_info->ip_len + datalen + 2;
	gw_buffer->data = (uint8 *)calloc(1, gw_buffer->size);
	incode_xtocs(gw_buffer->data, gw_info->gw_no, 8);
	get_frapp_type_to_str(gw_buffer->data+16, gw_info->zapp_type);
	incode_xtoc16(gw_buffer->data+18, gw_info->zpanid);
	incode_xtoc16(gw_buffer->data+22, gw_info->zchannel);
	incode_xtoc32(gw_buffer->data+26, gw_info->rand);
	memcpy(gw_buffer->data+34, gw_info->ipaddr, gw_info->ip_len);
	if(frbuffer != NULL)
	{
		memcpy(gw_buffer->data+34+gw_info->ip_len, data, datalen);
	}
	incode_xtocs(gw_buffer->data+gw_buffer->size-2, &datalen, 1);

	get_buffer_free(frbuffer);
	
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

	uint8 optdata_len;
	incode_ctoxs(&optdata_len, buffer+length-2, 2);

	gw_info_t *gw_info = (gw_info_t *)calloc(1, sizeof(gw_info_t));
	incode_ctoxs(gw_info->gw_no, buffer, 16);
	gw_info->zapp_type = get_frapp_type_from_str(buffer+16);
	incode_ctox16(&gw_info->zpanid, buffer+18);
	incode_ctox16(&gw_info->zchannel, buffer+22);
	incode_ctox32(&gw_info->rand, buffer+26);
	gw_info->ip_len = length-GATEWAY_BUFFER_FIX_SIZE-optdata_len-2;
	if(gw_info->ip_len < IP_ADDR_MAX_SIZE)
	{
		memset(gw_info->ipaddr, 0, sizeof(gw_info->ipaddr));
		memcpy(gw_info->ipaddr, buffer+34, gw_info->ip_len);
	}
	gw_info->zgw_opt = get_devopt_data_alloc(gw_info->zapp_type, 
		buffer+34+gw_info->ip_len, optdata_len);
	
	if(pthread_mutex_init(&gw_info->lock, NULL) != 0)
    {
		free(gw_info);
        return NULL;
    }
	
	gw_info->p_dev = NULL;
	gw_info->p_contain = NULL;
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

		fr_buffer_t *data_buffer = get_devopt_data_to_str(opt);
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


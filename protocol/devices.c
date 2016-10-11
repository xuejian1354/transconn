/*
 * devices.c
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
#include "devices.h"
#include <mincode.h>
#include <module/serial.h>
#include <protocol/request.h>

#ifdef __cplusplus
extern "C" {
#endif

static gw_info_t gw_info;

trbuffer_t *get_devopt_frame_alloc(uint8 devid, uint16 dataoff, uint16 datalen)
{
	trbuffer_t *frame = calloc(1, sizeof(trbuffer_t));

	frame->len = 8;
	frame->data = calloc(1, frame->len);

	*(frame->data) = devid;
	*(frame->data+1) = 0x03;
	*(frame->data+2) = (dataoff>>8) & 0xFF;
	*(frame->data+3) = dataoff & 0xFF;
	*(frame->data+4) = (datalen>>8) & 0xFF;
	*(frame->data+5) = datalen & 0xFF;

	uint32 crcval = crc16(frame->data, 6);
	*(frame->data+6) = (crcval>>8) & 0xFF;
	*(frame->data+7) = crcval & 0xFF;

	return frame;
}

void get_devopt_frame_free(trbuffer_t *buffer)
{
	if(buffer != NULL)
	{
		free(buffer->data);
	}

	free(buffer);
}

gw_info_t *get_gateway_info()
{
	return &gw_info;
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
		if(memcmp(t_dev->dev_no, m_dev->dev_no, sizeof(zidentify_no_t)))
		{
			pre_dev = t_dev;
			t_dev = t_dev->next;
		}
		else
		{
			if(memcmp(t_dev->data, m_dev->data, 4))
			{
				memcpy(t_dev->data, m_dev->data, 4);
				t_dev->ischange = 1;
			}
			else
			{
				t_dev->ischange = 0;
			}

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



dev_info_t *query_zdev_info(gw_info_t *gw_info, char *dev_no)
{
	dev_info_t *t_dev = gw_info->p_dev;


	while(t_dev != NULL)
	{
		if(memcmp(t_dev->dev_no, dev_no, sizeof(zidentify_no_t)))
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

int del_zdev_info(gw_info_t *gw_info, zidentify_no_t dev_no)
{
	dev_info_t *pre_dev =  NULL;
	dev_info_t *t_dev = gw_info->p_dev;


	while(t_dev != NULL)
	{
		if(memcmp(t_dev->dev_no, dev_no, sizeof(zidentify_no_t)))
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

			free(t_dev);
			return 0;
		}
	}

	return -1;
}


int add_zdevice_info(dev_info_t *m_dev)
{
	return add_zdev_info(get_gateway_info(), m_dev);
}

dev_info_t *query_zdevice_info(zidentify_no_t dev_no)
{
	dev_info_t *t_dev = get_gateway_info()->p_dev;

	while(t_dev != NULL)
	{
		if(memcmp(t_dev->dev_no, dev_no, sizeof(zidentify_no_t)))
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

int del_zdevice_info(zidentify_no_t dev_no)
{
	return del_zdev_info(get_gateway_info(), dev_no);
}

#ifdef __cplusplus
}
#endif

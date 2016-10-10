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
			if(memcmp(t_dev->dev_no, m_dev->dev_no, 3)
				|| t_dev->type != m_dev->type)
			{
				memcpy(t_dev->dev_no, m_dev->dev_no, 3);
				t_dev->type = m_dev->type;
			}

			if(!t_dev->ischange)
			{
				//t_dev->isdata_change = is_change;
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

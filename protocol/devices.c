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
#include <protocol/trframelysis.h>
#include <protocol/request.h>
#include <services/mevent.h>

#ifdef COMM_CLIENT
static gw_info_t gw_info;
#endif

#ifdef COMM_SERVER
static gw_list_t gw_list;
#endif

#ifdef COMM_CLIENT
gw_info_t *get_gateway_info()
{
	return &gw_info;
}
#endif


#ifdef COMM_SERVER
gw_list_t *get_gateway_list()
{
	return &gw_list;
}
#endif

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


#ifdef COMM_CLIENT
int add_zdevice_info(dev_info_t *m_dev)
{
	return add_zdev_info(get_gateway_info(), m_dev);
}

dev_info_t *query_zdevice_info(uint16 znet_addr)
{
	return query_zdev_info(get_gateway_info(), znet_addr);
}

dev_info_t *query_zdevice_info_with_sn(zidentify_no_t zidentify_no)
{
	dev_info_t *t_dev = get_gateway_info()->p_dev;


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
	return del_zdev_info(get_gateway_info(), znet_addr);
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
			t_gw->zapp_type = m_gw->zapp_type;
			t_gw->zpanid = m_gw->zpanid;
			t_gw->zchannel = m_gw->zchannel;
			t_gw->rand = m_gw->rand;
			t_gw->trans_type = m_gw->trans_type;
			t_gw->ip_len = m_gw->ip_len;
			set_devopt_data_fromopt(t_gw->zgw_opt, m_gw->zgw_opt);
			if(t_gw->ipaddr != m_gw->ipaddr)
			{
				memset(t_gw->ipaddr, 0, sizeof(t_gw->ipaddr));
				memcpy(t_gw->ipaddr, m_gw->ipaddr, m_gw->ip_len);
			}

			if(t_gw->serverip_addr !=m_gw->serverip_addr)
			{
				memset(t_gw->serverip_addr, 0, sizeof(t_gw->serverip_addr));
				memcpy(t_gw->serverip_addr, m_gw->serverip_addr, m_gw->serverip_len);
			}

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

			cli_contain_t *pre_contain = t_gw->p_contain;
			cli_contain_t *pcontain = t_gw->p_contain;
			
			while(pcontain != NULL)
			{
				pre_contain = pcontain;
				pcontain = pcontain->next;
				free(pre_contain);
			}

			free(t_gw);
			return 0;
		}
	}

	return -1;
}

int add_contain_info(cli_contain_t **contain, cli_contain_t *m_contain)
{
	cli_contain_t *pre_contain =  NULL;
	cli_contain_t *t_contain = *contain;

	if(m_contain == NULL || m_contain->p_cli == NULL)
	{
		return -1;
	}
	else
	{
		m_contain->next = NULL;
	}

	while(t_contain != NULL)
	{
		if(memcmp(t_contain->p_cli->cidentify_no, 
			m_contain->p_cli->cidentify_no, sizeof(cidentify_no_t)))
		{
			pre_contain = t_contain;
			t_contain = t_contain->next;
		}
		else
		{
			if(m_contain->p_cli->ip_len != 0
				&& t_contain->p_cli->ipaddr != m_contain->p_cli->ipaddr)
			{
				t_contain->p_cli->trans_type = m_contain->p_cli->trans_type;
				memset(t_contain->p_cli->ipaddr, 0, 
						sizeof(t_contain->p_cli->ipaddr));
				memcpy(t_contain->p_cli->ipaddr, 
						m_contain->p_cli->ipaddr, 
						m_contain->p_cli->ip_len);
				t_contain->p_cli->ip_len = m_contain->p_cli->ip_len;
			}
			
			if(m_contain->p_cli->serverip_len != 0
				&& t_contain->p_cli->serverip_addr != m_contain->p_cli->serverip_addr)
			{
				memset(t_contain->p_cli->serverip_addr, 0, 
					sizeof(t_contain->p_cli->serverip_addr));
				memcpy(t_contain->p_cli->serverip_addr, 
						m_contain->p_cli->serverip_addr, 
						m_contain->p_cli->serverip_len);
				t_contain->p_cli->serverip_len = m_contain->p_cli->serverip_len;
			}
			t_contain->p_cli->check_count = m_contain->p_cli->check_count;
			t_contain->p_cli->check_conn = m_contain->p_cli->check_conn;

			if(pre_contain != NULL)
			{
				pre_contain->next = t_contain->next;
				t_contain->next = *contain;
				*contain = t_contain;
			}
			
			return 1;
		}
	}

	m_contain->next = *contain;
	*contain = m_contain;

	return 0;
}



cli_contain_t *query_contain_info(cli_contain_t *contain, cidentify_no_t cidentify_no)
{
	cli_contain_t *t_contain = contain;


	while(t_contain != NULL)
	{
		if(memcmp(t_contain->p_cli->cidentify_no, cidentify_no, sizeof(cidentify_no_t)))
		{
			t_contain = t_contain->next;
		}
		else
		{
			return t_contain;
		}
	}

	return NULL;
}

int del_contain_info(cli_contain_t **contain, cidentify_no_t cidentify_no)
{
	cli_contain_t *pre_contain =  NULL;
	cli_contain_t *t_contain = *contain;


	while(t_contain != NULL)
	{
		if(memcmp(t_contain->p_cli->cidentify_no, cidentify_no, sizeof(cidentify_no_t)))
		{
			pre_contain = t_contain;
			t_contain = t_contain->next;
		}
		else
		{
			if(pre_contain != NULL)
			{
				pre_contain->next = t_contain->next;
			}
			else
			{
				*contain = t_contain->next;
			}

			free(t_contain);
			return 0;
		}
	}

	return -1;
}

#endif

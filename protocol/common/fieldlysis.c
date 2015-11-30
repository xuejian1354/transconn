/*
 * fieldlysis.c
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
#include "fieldlysis.h"

static char action_str[6];

char *get_action_to_str(trans_action_t action)
{
	bzero(action_str, sizeof(action_str));
	sprintf(action_str, "%d", action);
	return action_str;
}

trfield_device_t *get_trfield_device_alloc(char *name,
	sn_t dev_sn, char *dev_type, int znet_status, char *dev_data)
{
	if(name == NULL || dev_sn == NULL || dev_type == NULL || dev_data == NULL)
	{
		return NULL;
	}
	
	trfield_device_t *device = calloc(1, sizeof(trfield_device_t));
	STRS_MEMCPY(device->name, name, sizeof(device->name), strlen(name));
	STRS_MEMCPY(device->dev_sn, dev_sn, sizeof(device->dev_sn), strlen(dev_sn));
	device->dev_type = get_frapp_type_from_str(dev_type);
	device->znet_status = znet_status;
	STRS_MEMCPY(device->dev_data, dev_data, sizeof(device->dev_data), strlen(dev_data));

	return device;
}

trfr_tocolreq_t *get_trfr_tocolreq_alloc(char *protocol, char *random)
{
	if(protocol == NULL)
	{
		return NULL;
	}

	trfr_tocolreq_t *tocolreq = calloc(1, sizeof(trfr_tocolreq_t));
	tocolreq->action = ACTION_TOCOLREQ;
	STRS_MEMCPY(tocolreq->protocol, 
		protocol, sizeof(tocolreq->protocol), strlen(protocol));

	char *gen_md;
	if(random == NULL)
	{
		gen_md = get_md5(get_system_time(), 2);
	}
	else
	{
		gen_md = random;
	}

	STRS_MEMCPY(tocolreq->random, gen_md, sizeof(tocolreq->random), strlen(gen_md));
	return tocolreq;
}

void get_trfr_tocolreq_free(trfr_tocolreq_t *tocolreq)
{
	free(tocolreq);
}

trfr_report_t *get_trfr_report_alloc(sn_t gw_sn, trfield_device_t **devices, int dev_size, char *random)
{
	if(gw_sn == NULL || devices == NULL || dev_size <= 0)
	{
		return NULL;
	}

	int i;
	trfr_report_t *report = calloc(1, sizeof(trfr_report_t));
	report->action = ACTION_REPORT;
	STRS_MEMCPY(report->gw_sn, gw_sn, sizeof(report->gw_sn), strlen(gw_sn));
	report->devices = devices;
	report->dev_size = dev_size;

	char *gen_md;
	if(random == NULL)
	{
		gen_md = get_md5(get_system_time(), 2);
	}
	else
	{
		gen_md = random;
	}

	STRS_MEMCPY(report->random, gen_md, sizeof(report->random), strlen(gen_md));
	return report;
}

void get_trfr_report_free(trfr_report_t *report)
{
	if(report != NULL)
	{
		if(report->devices != NULL && report->dev_size > 0)
		{
			int i = report->dev_size;
			while(i > 0)
			{
				free(*(report->devices+i-1));
				i--;
			}
			free(report->devices);
		}
		free(report);
	}
}

trfr_check_t *get_trfr_check_alloc(sn_t gw_sn, sn_t dev_sns[], int sn_size, char *code_check, char *code_data, char *random)
{
	if(gw_sn == NULL ||  code_check == NULL || code_data == NULL)
	{
		return NULL;
	}

	int i;
	trfr_check_t *check = calloc(1, sizeof(trfr_check_t));
	check->action = ACTION_CHECK;
	STRS_MEMCPY(check->gw_sn, gw_sn, sizeof(check->gw_sn), strlen(gw_sn));
	check->dev_sns = dev_sns;
	check->sn_size = sn_size;
	STRS_MEMCPY(check->code.code_check, 
		code_check, sizeof(check->code.code_check), strlen(code_check));
	STRS_MEMCPY(check->code.code_data, 
		code_data, sizeof(check->code.code_data), strlen(code_data));

	char *gen_md;
	if(random == NULL)
	{
		gen_md = get_md5(get_system_time(), 2);
	}
	else
	{
		gen_md = random;
	}

	STRS_MEMCPY(check->random, gen_md, sizeof(check->random), strlen(gen_md));

	return check;
}

void get_trfr_check_free(trfr_check_t *check)
{
	if(check != NULL)
	{
		if(check->dev_sns!= NULL)
		{
			free(check->dev_sns);
		}

		free(check);
	}
}

trfr_respond_t *get_trfr_respond_alloc(sn_t gw_sn, sn_t dev_sn, char *dev_data, char *random)
{
	if(gw_sn == NULL || dev_sn == NULL || dev_data == NULL)
	{
		return NULL;
	}

	trfr_respond_t *respond = calloc(1, sizeof(trfr_respond_t));
	respond->action = ACTION_RESPOND;
	STRS_MEMCPY(respond->gw_sn, gw_sn, sizeof(respond->gw_sn), strlen(gw_sn));
	STRS_MEMCPY(respond->dev_sn, dev_sn, sizeof(respond->dev_sn), strlen(dev_sn));
	STRS_MEMCPY(respond->dev_data, 
		dev_data, sizeof(respond->dev_data), strlen(dev_data));

	char *gen_md;
	if(random == NULL)
	{
		gen_md = get_md5(get_system_time(), 2);
	}
	else
	{
		gen_md = random;
	}

	STRS_MEMCPY(respond->random, gen_md, sizeof(respond->random), strlen(gen_md));
	
	return respond;
}

void get_trfr_respond_free(trfr_respond_t *respond)
{
	free(respond);
}

trfr_refresh_t *get_trfr_refresh_alloc(sn_t gw_sn, sn_t dev_sns[], int sn_size, char *random)
{
	if(gw_sn == NULL || dev_sns == NULL || sn_size <=0)
	{
		return NULL;
	}

	int i;
	trfr_refresh_t *refresh = calloc(1, sizeof(trfr_refresh_t));
	refresh->action = ACTION_REFRESH;
	STRS_MEMCPY(refresh->gw_sn, gw_sn, sizeof(refresh->gw_sn), strlen(gw_sn));
	refresh->dev_sns = dev_sns;
	refresh->sn_size = sn_size;

	char *gen_md;
	if(random == NULL)
	{
		gen_md = get_md5(get_system_time(), 2);
	}
	else
	{
		gen_md = random;
	}

	STRS_MEMCPY(refresh->random, gen_md, sizeof(refresh->random), strlen(gen_md));

	return refresh;
}

void get_trfr_refresh_free(trfr_refresh_t *refresh)
{
	if(refresh != NULL)
	{
		if(refresh->dev_sns!= NULL)
		{
			free(refresh->dev_sns);
		}

		free(refresh);
	}
}

trfr_control_t *get_trfr_control_alloc(sn_t gw_sn, sn_t dev_sns[], int sn_size, char *cmd, char *random)
{
	if(gw_sn == NULL || dev_sns == NULL || sn_size <= 0 || cmd == NULL)
	{
		return NULL;
	}

	int i;
	trfr_control_t *control = calloc(1, sizeof(trfr_control_t));
	control->action = ACTION_CONTROL;
	STRS_MEMCPY(control->gw_sn, gw_sn, sizeof(control->gw_sn), strlen(gw_sn));
	control->dev_sns = dev_sns;
	control->sn_size = sn_size;
	STRS_MEMCPY(control->cmd, cmd, sizeof(control->cmd), strlen(cmd));

	char *gen_md;
	if(random == NULL)
	{
		gen_md = get_md5(get_system_time(), 2);
	}
	else
	{
		gen_md = random;
	}

	STRS_MEMCPY(control->random, gen_md, sizeof(control->random), strlen(gen_md));

	return control;
}

void get_trfr_control_free(trfr_control_t *control)
{
	if(control != NULL)
	{
		if(control->dev_sns!= NULL)
		{
			free(control->dev_sns);
		}

		free(control);
	}
}

trfr_tocolres_t *get_trfr_tocolres_alloc(trans_action_t req_action, char *random)
{
	trfr_tocolres_t *tocolres = calloc(1, sizeof(trfr_tocolres_t));
	tocolres->action = ACTION_TOCOLRES;
	tocolres->req_action = req_action;

	char *gen_md;
	if(random == NULL)
	{
		gen_md = get_md5(get_system_time(), 2);
	}
	else
	{
		gen_md = random;
	}

	STRS_MEMCPY(tocolres->random, gen_md, sizeof(tocolres->random), strlen(gen_md));

	return tocolres;
}

void get_trfr_tocolres_free(trfr_tocolres_t *tocolres)
{
	free(tocolres);
}


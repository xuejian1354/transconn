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
#include <protocol/devices.h>

#ifdef __cplusplus
extern "C" {
#endif

static char action_str[6];

#ifdef ZH_TYPE_NAME
static zh_el_t zh_tables[TN_NUMS] =
{
	{ FRAPP_CONNECTOR, "网关" },
	{ FRAPP_CAMERA, "摄像头" },
	{ FRAPP_TEMP, "温度" },
	{ FRAPP_HUMI, "湿度" },
	{ FRAPP_ILLUMI, "光照强度" },
	{ FRAPP_RAINFALL, "降雨量" },
	{ FRAPP_AIRSPEED, "风速" },
	{ FRAPP_AIRDIRECTION, "风向" },
	{ FRAPP_AMMONIA, "氨气" },
	{ FRAPP_NITROGEN, "氮气" },
	{ FRAPP_HYDROTHION, "硫化氢" },
	{ FRAPP_DUST, "粉尘" },
	{ FRAPP_NOISE, "噪声" },
	{ FRAPP_PRESSURE, "气压" },
	{ FRAPP_DIOXIDE, "二氧化碳浓度" },
	{ FRAPP_WATER_DIOXIDE, "溶解氧含量" },
	{ FRAPP_WATER_NITROGEN, "氨氮含量" },
	{ FRAPP_WATER_NITRITE, "亚硝酸盐含量" },
	{ FRAPP_WATER_TEMP, "水温" },
	{ FRAPP_WATER_PH, "水PH值" },
	{ FRAPP_SOIL_TEMP, "土壤温度" },
	{ FRAPP_SOIL_MOISTURE, "土壤水分" },
	{ FRAPP_SOIL_SALINITY, "土壤盐分" },
	{ FRAPP_SOIL_PH, "土壤PH值" },
	{ FRAPP_FERTILIZER_CTRL, "施肥机" },
	{ FRAPP_AERATOR_CTRL, "增氧机" },
	{ FRAPP_ELECTRIC_CTRL, "电机" },
	{ FRAPP_FAN_CTRL, "风机" },
	{ FRAPP_PUMP_CTRL, "水泵" },
	{ FRAPP_VALVE_CTRL, "阀门" },
	{ FRAPP_SOLENOID_CTRL, "电磁阀" },
	{ FRAPP_SHUTTER_CTRL, "卷帘机" },
};

#else
static zh_el_t zh_tables[TN_NUMS] =
{
	{ FRAPP_CONNECTOR, "Gateway" },
	{ FRAPP_CAMERA, "Camera" },
	{ FRAPP_TEMP, "Temperature" },
	{ FRAPP_HUMI, "Humidity" },
	{ FRAPP_ILLUMI, "Illumination" },
	{ FRAPP_RAINFALL, "RainFall" },
	{ FRAPP_AIRSPEED, "AirSpeed" },
	{ FRAPP_AIRDIRECTION, "AirDirection" },
	{ FRAPP_AMMONIA, "Ammonia" },
	{ FRAPP_NITROGEN, "Nitrogen" },
	{ FRAPP_HYDROTHION, "Hydrothion" },
	{ FRAPP_DUST, "Dust" },
	{ FRAPP_NOISE, "Noise" },
	{ FRAPP_PRESSURE, "Pressure" },
	{ FRAPP_DIOXIDE, "Dioxide" },
	{ FRAPP_WATER_DIOXIDE, "WaterDioxide" },
	{ FRAPP_WATER_NITROGEN, "WaterNitrogen" },
	{ FRAPP_WATER_NITRITE, "WaterNitrite" },
	{ FRAPP_WATER_TEMP, "WaterTemperature" },
	{ FRAPP_WATER_PH, "WaterPH" },
	{ FRAPP_SOIL_TEMP, "SoilTemperature" },
	{ FRAPP_SOIL_MOISTURE, "SoilMoisture" },
	{ FRAPP_SOIL_SALINITY, "SoilSalinity" },
	{ FRAPP_SOIL_PH, "SoilPH" },
	{ FRAPP_FERTILIZER_CTRL, "FertilizerCtrl" },
	{ FRAPP_AERATOR_CTRL, "AeratorCtrl" },
	{ FRAPP_ELECTRIC_CTRL, "ElectricCtrl" },
	{ FRAPP_FAN_CTRL, "FanCtrl" },
	{ FRAPP_PUMP_CTRL, "PumpCtrl" },
	{ FRAPP_VALVE_CTRL, "ValveCtrl" },
	{ FRAPP_SOLENOID_CTRL, "SolenoidCtrl" },
	{ FRAPP_SHUTTER_CTRL, "ShutterCtrl" },
};
#endif

static char mix_type_name[64];

char *get_name(type_name_t tn)
{
	return (char *)zh_tables[tn].val;
}

char *get_name_from_type(fr_app_type_t type)
{
	int i;
	for(i=0; i<TN_NUMS; i++)
	{
		if(type == zh_tables[i].type)
		{
			return (char *)zh_tables[i].val;
		}
	}

	return (char *)zh_tables[TN_NUMS].val;
}

char *get_mix_name(fr_app_type_t type, uint8 s1, uint8 s2)
{
	memset(mix_type_name, 0, sizeof(mix_type_name));
	sprintf(mix_type_name, "%s%02X%02X", get_name_from_type(type), s1, s2);
	return mix_type_name;
}

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
	
	trfield_device_t *device = (trfield_device_t *)calloc(1, sizeof(trfield_device_t));
	STRS_MEMCPY(device->name, name, sizeof(device->name), strlen(name));
	STRS_MEMCPY(device->dev_sn, dev_sn, sizeof(device->dev_sn), strlen(dev_sn));
	//device->dev_type = get_frapp_type_from_str(dev_type);
	device->znet_status = znet_status;
	STRS_MEMCPY(device->dev_data, dev_data, sizeof(device->dev_data), strlen(dev_data));

	return device;
}

trfield_ctrl_t *get_trfield_ctrl_alloc(sn_t dev_sn, char *cmd)
{
	if(dev_sn == NULL || cmd == NULL)
	{
		return NULL;
	}
	
	trfield_ctrl_t *ctrl = (trfield_ctrl_t *)calloc(1, sizeof(trfield_ctrl_t));

	STRS_MEMCPY(ctrl->dev_sn, dev_sn, sizeof(ctrl->dev_sn), strlen(dev_sn));
	STRS_MEMCPY(ctrl->cmd, cmd, sizeof(ctrl->cmd), strlen(cmd));

	return ctrl;
}

trfield_obj_t *get_trfield_obj_alloc(char *owner, char *custom)
{
	if(owner == NULL || custom == NULL)
	{
		return NULL;
	}

	trfield_obj_t *obj = (trfield_obj_t *)calloc(1, sizeof(trfield_obj_t));
	STRS_MEMCPY(obj->owner, owner, sizeof(obj->owner), strlen(owner));
	STRS_MEMCPY(obj->custom, custom, sizeof(obj->custom), strlen(custom));

	return obj;
}

trfr_tocolreq_t *get_trfr_tocolreq_alloc(trfield_obj_t *obj, sn_t gw_sn, char *protocol, char *random)
{
	if(protocol == NULL)
	{
		return NULL;
	}

	trfr_tocolreq_t *tocolreq = (trfr_tocolreq_t *)calloc(1, sizeof(trfr_tocolreq_t));
	tocolreq->action = ACTION_TOCOLREQ;
	tocolreq->obj = obj;
	STRS_MEMCPY(tocolreq->gw_sn, gw_sn, sizeof(tocolreq->gw_sn), strlen(gw_sn));
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
	if(tocolreq != NULL)
	{
		free(tocolreq->obj);
	}
	free(tocolreq);
}

trfr_report_t *get_trfr_report_alloc(trfield_obj_t *obj, sn_t gw_sn, trfield_device_t **devices, int dev_size, char *random)
{
	if(gw_sn == NULL || devices == NULL || dev_size <= 0)
	{
		return NULL;
	}

	trfr_report_t *report = (trfr_report_t *)calloc(1, sizeof(trfr_report_t));
	report->action = ACTION_REPORT;
	report->obj = obj;
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
		free(report->obj);

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

trfr_check_t *get_trfr_check_alloc(trfield_obj_t *obj, sn_t gw_sn, sn_t dev_sns[], int sn_size, char *code_check, char *code_data, char *random)
{
	if(gw_sn == NULL ||  code_check == NULL || code_data == NULL)
	{
		return NULL;
	}

	int i;
	trfr_check_t *check = (trfr_check_t *)calloc(1, sizeof(trfr_check_t));
	check->action = ACTION_CHECK;
	check->obj = obj;
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
		free(check->obj);

		if(check->dev_sns!= NULL)
		{
			free(check->dev_sns);
		}

		free(check);
	}
}

trfr_respond_t *get_trfr_respond_alloc(trfield_obj_t *obj, sn_t gw_sn, sn_t dev_sn, char *dev_data, char *random)
{
	if(gw_sn == NULL || dev_sn == NULL || dev_data == NULL)
	{
		return NULL;
	}

	trfr_respond_t *respond = (trfr_respond_t *)calloc(1, sizeof(trfr_respond_t));
	respond->action = ACTION_RESPOND;
	respond->obj = obj;
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
	if(respond != NULL)
	{
		free(respond->obj);
	}
	free(respond);
}

trfr_refresh_t *get_trfr_refresh_alloc(trfield_obj_t *obj, sn_t gw_sn, sn_t dev_sns[], int sn_size, char *random)
{
	if(gw_sn == NULL)
	{
		return NULL;
	}

	int i;
	trfr_refresh_t *refresh = (trfr_refresh_t *)calloc(1, sizeof(trfr_refresh_t));
	refresh->action = ACTION_REFRESH;
	refresh->obj = obj;
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
		free(refresh->obj);
		if(refresh->dev_sns!= NULL)
		{
			free(refresh->dev_sns);
		}

		free(refresh);
	}
}

trfr_control_t *get_trfr_control_alloc(trfield_obj_t *obj, sn_t gw_sn, trfield_ctrl_t **ctrls, int ctrl_size, char *random)
{
	if(ctrls == NULL || ctrl_size <= 0)
	{
		return NULL;
	}

	int i;
	trfr_control_t *control = (trfr_control_t *)calloc(1, sizeof(trfr_control_t));
	control->action = ACTION_CONTROL;
	control->obj = obj;
	if(gw_sn == NULL)
	{
#ifdef COMM_TARGET
		incode_xtocs(control->gw_sn, get_gateway_info()->gw_no, sizeof(zidentify_no_t));
#endif
	}
	else
	{
		STRS_MEMCPY(control->gw_sn, gw_sn, sizeof(control->gw_sn), strlen(gw_sn));
	}
	control->ctrls= ctrls;
	control->ctrl_size= ctrl_size;

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
		free(control->obj);
		if(control->ctrls != NULL && control->ctrl_size > 0)
		{
			int i=0;
			while(i < control->ctrl_size)
			{
				trfield_ctrl_t *ctrl = *(control->ctrls+i);
				free(ctrl);

				i++;
			}
			free(control->ctrls);
		}
		free(control);
	}
}

trfr_tocolres_t *get_trfr_tocolres_alloc(trfield_obj_t *obj, trans_action_t req_action, char *info, char *random)
{
	trfr_tocolres_t *tocolres = (trfr_tocolres_t *)calloc(1, sizeof(trfr_tocolres_t));
	tocolres->action = ACTION_TOCOLRES;
	tocolres->obj = obj;
	tocolres->req_action = req_action;

	if(info != NULL)
	{
		STRS_MEMCPY(tocolres->info, info, sizeof(tocolres->info), strlen(info));
	}

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
	if(tocolres != NULL)
	{
		free(tocolres->obj);
	}
	free(tocolres);
}

#ifdef __cplusplus
}
#endif


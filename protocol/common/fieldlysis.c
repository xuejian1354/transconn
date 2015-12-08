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

#ifdef ZH_TYPE_NAME
static zh_el_t zh_tables[TN_NUMS+1] =
{
    { FRAPP_CONNECTOR, "网关" },
    { FRAPP_LIGHTSWITCH_ONE, "一位开关" },
    { FRAPP_LIGHTSWITCH_TWO, "二位开关" },
    { FRAPP_LIGHTSWITCH_THREE, "三位开关" },
    { FRAPP_LIGHTSWITCH_FOUR, "四位开关" },
    { FRAPP_HUELIGHT, "调色灯" },
    { FRAPP_ALARM, "报警器" },
    { FRAPP_IR_DETECTION, "人体感应" },
    { FRAPP_DOOR_SENSOR, "门磁" },
    { FRAPP_ENVDETECTION, "PM2.5检测" },
    { FRAPP_IR_RELAY, "红外转发" },
    { FRAPP_AIRCONTROLLER, "环境检测仪" },
    { FRAPP_RELAYSOCKET, "中继开关" },
    { FRAPP_HUMITURE_DETECTION, "温湿度" },
    { FRAPP_SOLENOID_VALVE, "电磁阀" },
    { FRAPP_LAMPSWITCH, "灯开关" },
    { FRAPP_PROJECTOR, "投影仪" },
    { FRAPP_AIRCONDITION, "空调" },
    { FRAPP_CURTAIN, "窗帘" },
    { FRAPP_DOORLOCK, "门禁" },
    { FRAPP_NONE, "" }
};

#else
static zh_el_t zh_tables[TN_NUMS+1] =
{
    { FRAPP_CONNECTOR, "Gateway" },
    { FRAPP_LIGHTSWITCH_ONE, "LightSwitchI" },
    { FRAPP_LIGHTSWITCH_TWO, "LightSwitchII" },
    { FRAPP_LIGHTSWITCH_THREE, "LightSwitchIII" },
    { FRAPP_LIGHTSWITCH_FOUR, "LightSwitchIV" },
    { FRAPP_HUELIGHT, "HueLight" },
    { FRAPP_ALARM, "Alarm" },
    { FRAPP_IR_DETECTION, "InfraredMoving" },
    { FRAPP_DOOR_SENSOR, "DoorSensor" },
    { FRAPP_ENVDETECTION, "PM2.5" },
    { FRAPP_IR_RELAY, "InfraredControl" },
    { FRAPP_AIRCONTROLLER, "EnvControl" },
    { FRAPP_RELAYSOCKET, "NetRelay" },
    { FRAPP_HUMITURE_DETECTION, "Humiture" },
    { FRAPP_SOLENOID_VALVE, "Valve" },
    { FRAPP_LAMPSWITCH, "LampSwitch" },
    { FRAPP_PROJECTOR, "Projector" },
    { FRAPP_AIRCONDITION, "AirCondition" },
    { FRAPP_CURTAIN, "Curtain" },
    { FRAPP_DOORLOCK, "Doorlock" },
    { FRAPP_NONE, "" }
};
#endif

static char mix_type_name[64];

char *get_name(type_name_t tn)
{
	return zh_tables[tn].val;
}

char *get_name_from_type(fr_app_type_t type)
{
	int i;
	for(i=0; i<TN_NUMS; i++)
	{
		if(type == zh_tables[i].type)
		{
			return zh_tables[i].val;
		}
	}

	return zh_tables[TN_NUMS].val;
}

char *get_mix_name(fr_app_type_t type, uint8 s1, uint8 s2)
{
	memset(mix_type_name, 0, sizeof(mix_type_name));
	sprintf(mix_type_name, "%s%02X%02X", get_name_from_type(type), s1, s2);
	return mix_type_name;
}

fr_app_type_t get_frapp_type_from_str(char *app_type)
{
	if(app_type == NULL)
	{
		return FRAPP_NONE;
	}
	
	if(!strncmp(FR_APP_CONNECTOR, app_type, 2))
	{
		return FRAPP_CONNECTOR;
	}
	else if(!strncmp(FR_APP_LIGHTSWITCH_ONE, app_type, 2))
	{
		return FRAPP_LIGHTSWITCH_ONE;
	}
	else if(!strncmp(FR_APP_LIGHTSWITCH_TWO, app_type, 2))
	{
		return FRAPP_LIGHTSWITCH_TWO;
	}
	else if(!strncmp(FR_APP_LIGHTSWITCH_THREE, app_type, 2))
	{
		return  FRAPP_LIGHTSWITCH_THREE;
	}
	else if(!strncmp(FR_APP_LIGHTSWITCH_FOUR, app_type, 2))
	{
		return FRAPP_LIGHTSWITCH_FOUR;
	}
	else if(!strncmp(FR_APP_HUELIGHT, app_type, 2))
	{
		return FRAPP_HUELIGHT;
	}
	else if(!strncmp(FR_APP_ALARM, app_type, 2))
	{
		return FRAPP_ALARM;
	}
	else if(!strncmp(FR_APP_IR_DETECTION, app_type, 2))
	{
		return FRAPP_IR_DETECTION;
	}
	else if(!strncmp(FR_APP_DOOR_SENSOR, app_type, 2))
	{
		return FRAPP_DOOR_SENSOR;
	}
	else if(!strncmp(FR_APP_ENVDETECTION, app_type, 2))
	{
		return FRAPP_ENVDETECTION;
	}
	else if(!strncmp(FR_APP_IR_RELAY, app_type, 2))
	{
		return FRAPP_IR_RELAY;
	}
	else if(!strncmp(FR_APP_AIRCONTROLLER, app_type, 2))
	{
		return FRAPP_AIRCONTROLLER;
	}
	else if(!strncmp(FR_APP_RELAYSOCKET, app_type, 2))
	{
		return FRAPP_RELAYSOCKET;
	}
	else if(!strncmp(FR_APP_LIGHTDETECT, app_type, 2))
	{
		return FRAPP_LIGHTDETECT;
	}
	else if(!strncmp(FR_APP_HUMITURE_DETECTION, app_type, 2))
	{
		return FRAPP_HUMITURE_DETECTION;
	}
	else if(!strncmp(FR_APP_SOLENOID_VALVE, app_type, 2))
	{
		return FRAPP_SOLENOID_VALVE;
	}
	else if(!strncmp(FR_APP_LAMPSWITCH, app_type, 2))
	{
		return FRAPP_LAMPSWITCH;
	}
	else if(!strncmp(FR_APP_PROJECTOR, app_type, 2))
	{
		return FRAPP_PROJECTOR;
	}
	else if(!strncmp(FR_APP_AIRCONDITION, app_type, 2))
	{
		return FRAPP_AIRCONDITION;
	}
	else if(!strncmp(FR_APP_CURTAIN, app_type, 2))
	{
		return FRAPP_CURTAIN;
	}
	else if(!strncmp(FR_APP_DOORLOCK, app_type, 2))
	{
		return FRAPP_DOORLOCK;
	}

	return FRAPP_NONE;
}

char *get_frapp_type_to_str(fr_app_type_t app_type)
{
	switch(app_type)
	{
	case FRAPP_CONNECTOR: return FR_APP_CONNECTOR;
	case FRAPP_LIGHTSWITCH_ONE: return FR_APP_LIGHTSWITCH_ONE;
	case FRAPP_LIGHTSWITCH_TWO: return FR_APP_LIGHTSWITCH_TWO;
	case FRAPP_LIGHTSWITCH_THREE: return FR_APP_LIGHTSWITCH_THREE;
	case FRAPP_LIGHTSWITCH_FOUR: return FR_APP_LIGHTSWITCH_FOUR;
	case FRAPP_HUELIGHT: return FR_APP_HUELIGHT;
	case FRAPP_ALARM: return FR_APP_ALARM;
	case FRAPP_IR_DETECTION: return FR_APP_IR_DETECTION;
	case FRAPP_DOOR_SENSOR: return FR_APP_DOOR_SENSOR;
	case FRAPP_ENVDETECTION: return FR_APP_ENVDETECTION;
	case FRAPP_IR_RELAY: return FR_APP_IR_RELAY;
	case FRAPP_AIRCONTROLLER: return FR_APP_AIRCONTROLLER;
	case FRAPP_RELAYSOCKET: return FR_APP_RELAYSOCKET;
	case FRAPP_LIGHTDETECT: return FR_APP_LIGHTDETECT;
	case FRAPP_HUMITURE_DETECTION: return FR_APP_HUMITURE_DETECTION;
	case FRAPP_SOLENOID_VALVE: return FR_APP_SOLENOID_VALVE;
	case FRAPP_LAMPSWITCH: return FR_APP_LAMPSWITCH;
	case FRAPP_PROJECTOR: return FR_APP_PROJECTOR;
	case FRAPP_AIRCONDITION: return FR_APP_AIRCONDITION;
	case FRAPP_CURTAIN: return FR_APP_CURTAIN;
	case FRAPP_DOORLOCK: return FR_APP_DOORLOCK;
	}

	return "FF";
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
	
	trfield_device_t *device = calloc(1, sizeof(trfield_device_t));
	STRS_MEMCPY(device->name, name, sizeof(device->name), strlen(name));
	STRS_MEMCPY(device->dev_sn, dev_sn, sizeof(device->dev_sn), strlen(dev_sn));
	device->dev_type = get_frapp_type_from_str(dev_type);
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
	
	trfield_ctrl_t *ctrl = calloc(1, sizeof(trfield_ctrl_t));

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

	trfield_obj_t *obj = calloc(1, sizeof(trfield_obj_t));
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

	trfr_tocolreq_t *tocolreq = calloc(1, sizeof(trfr_tocolreq_t));
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

	trfr_report_t *report = calloc(1, sizeof(trfr_report_t));
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
	trfr_check_t *check = calloc(1, sizeof(trfr_check_t));
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

	trfr_respond_t *respond = calloc(1, sizeof(trfr_respond_t));
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
	if(gw_sn == NULL || dev_sns == NULL || sn_size <=0)
	{
		return NULL;
	}

	int i;
	trfr_refresh_t *refresh = calloc(1, sizeof(trfr_refresh_t));
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
	if(gw_sn == NULL || ctrls == NULL || ctrl_size <= 0)
	{
		return NULL;
	}

	int i;
	trfr_control_t *control = calloc(1, sizeof(trfr_control_t));
	control->action = ACTION_CONTROL;
	control->obj = obj;
	STRS_MEMCPY(control->gw_sn, gw_sn, sizeof(control->gw_sn), strlen(gw_sn));
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
	trfr_tocolres_t *tocolres = calloc(1, sizeof(trfr_tocolres_t));
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


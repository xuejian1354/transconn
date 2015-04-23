/*
 * devopt.c
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
#include "devopt.h"

dev_opt_t *get_devopt_data_alloc(fr_app_type_t type, uint8 *data, int len)
{
	dev_opt_t *opt = NULL;
	
	switch(type)
	{
	case FRAPP_LIGHTSWITCH_ONE: 
		opt = calloc(1, sizeof(dev_opt_t));
		opt->type = type;
		opt->common.method = DEV_CONTROL_BOTH;
		if(len >= 2)
		{
			opt->device.lightswitch.nums = 1;
			incode_ctoxs(opt->device.lightswitch.data.one, data, 2);
		}
		else
		{
			opt->device.lightswitch.data.one[0] = 0;
		}
		return opt;
		
	case FRAPP_LIGHTSWITCH_TWO: 
		opt = calloc(1, sizeof(dev_opt_t));
		opt->type = type;
		opt->common.method = DEV_CONTROL_BOTH;
		if(len >= 4)
		{
			opt->device.lightswitch.nums = 2;
			incode_ctoxs(opt->device.lightswitch.data.two, data, 4);
		}
		else
		{
			opt->device.lightswitch.data.two[0] = 0;
			opt->device.lightswitch.data.two[1] = 0;
		}
		return opt;
		
	case FRAPP_LIGHTSWITCH_THREE: 
		opt = calloc(1, sizeof(dev_opt_t));
		opt->type = type;
		opt->common.method = DEV_CONTROL_BOTH;
		if(len >= 6)
		{
			opt->device.lightswitch.nums = 3;
			incode_ctoxs(opt->device.lightswitch.data.three, data, 6);
		}
		else
		{
			opt->device.lightswitch.data.three[0] = 0;
			opt->device.lightswitch.data.three[1] = 0;
			opt->device.lightswitch.data.three[2] = 0;
		}
		return opt;
		
	case FRAPP_LIGHTSWITCH_FOUR: 
		opt = calloc(1, sizeof(dev_opt_t));
		opt->type = type;
		opt->common.method = DEV_CONTROL_BOTH;
		if(len >= 8)
		{
			opt->device.lightswitch.nums = 4;
			incode_ctoxs(opt->device.lightswitch.data.four, data, 8);
		}
		else
		{
			opt->device.lightswitch.data.four[0] = 0;
			opt->device.lightswitch.data.four[1] = 0;
			opt->device.lightswitch.data.four[2] = 0;
			opt->device.lightswitch.data.four[4] = 0;
		}
		return opt;
		
	case FRAPP_ALARM: 
		opt = calloc(1, sizeof(dev_opt_t));
		opt->type = type;
		opt->common.method = DEV_CONTROL_BOTH;
		if(len >= 2)
		{
			incode_ctoxs(opt->device.alarm.data, data, 2);
		}
		else
		{
			opt->device.alarm.data[0] = 0;
		}
		return opt;
		
	case FRAPP_IR_DETECTION: 
		opt = calloc(1, sizeof(dev_opt_t));
		opt->type = type;
		opt->common.power = DEV_DETECT_NORMAL;
		if(len >= 2)
		{
			opt->device.irdetect.setting = 0;
			incode_ctoxs(opt->device.irdetect.status, data, 2);
		}
		else
		{
			opt->device.irdetect.setting = 0;
			opt->device.irdetect.status[0] = 0;
		}
		return opt;
		
	case FRAPP_DOOR_SENSOR: 
		opt = calloc(1, sizeof(dev_opt_t));
		opt->type = type;
		opt->common.power = DEV_DETECT_NORMAL;
		if(len >= 2)
		{
			opt->device.doorsensor.setting = 0;
			incode_ctoxs(opt->device.doorsensor.status, data, 2);
		}
		else
		{
			opt->device.doorsensor.setting = 0;
			opt->device.doorsensor.status[0] = 0;
		}
		return opt;
		
	case FRAPP_IR_RELAY: 
		return opt;

	default: return NULL;
	}
}


void get_devopt_data_free(dev_opt_t *opt)
{
	free(opt);
}

int set_devopt_data(dev_opt_t *opt, uint8 *data, int len)
{	
	switch(opt->type)
	{
	case FRAPP_LIGHTSWITCH_ONE: 
		if(len >= 1)
		{
			opt->device.lightswitch.data.one[0] = data[0];
		}
		else
		{
			return -1;
		}
		break;
		
	case FRAPP_LIGHTSWITCH_TWO: 
		if(len >= 2)
		{
			opt->device.lightswitch.data.two[0] = data[0]; 
			opt->device.lightswitch.data.two[1] = data[1];
		}
		else
		{
			return -1;
		}
		break;
		
	case FRAPP_LIGHTSWITCH_THREE: 
		if(len >= 3)
		{
			opt->device.lightswitch.data.three[0] = data[0];
			opt->device.lightswitch.data.three[1] = data[1];
			opt->device.lightswitch.data.three[2] = data[2];
		}
		else
		{
			return -1;
		}
		break;
		
	case FRAPP_LIGHTSWITCH_FOUR: 
		if(len >= 4)
		{
			opt->device.lightswitch.data.four[0] = data[0];
			opt->device.lightswitch.data.four[1] = data[1];
			opt->device.lightswitch.data.four[2] = data[2];
			opt->device.lightswitch.data.four[3] = data[3];
		}
		else
		{
			return -1;
		}
		break;
		
	case FRAPP_ALARM: 
		if(len >=1)
		{
			opt->device.alarm.data[0] = data[0];
		}
		else
		{
			return -1;
		}
		break;
		
	case FRAPP_IR_DETECTION: 
		if(len >= 1)
		{
			opt->device.irdetect.status[0] = data[0];
		}
		else
		{
			return -1;
		}
		break;
		
	case FRAPP_DOOR_SENSOR: 
		if(len >= 1)
		{
			opt->device.doorsensor.status[0] = data[0];
		}
		else
		{
			return -1;
		}
		break;

	default: return -1;
	}

	return 0;
}

int set_devopt_data_fromopt(dev_opt_t *dst, dev_opt_t *src)
{	
	if(dst->type != src->type)
	{
		return -1;
	}
	
	switch(dst->type)
	{
	case FRAPP_LIGHTSWITCH_ONE: 
		dst->device.lightswitch.data.one[0] = src->device.lightswitch.data.one[0];
		break;
		
	case FRAPP_LIGHTSWITCH_TWO: 
		dst->device.lightswitch.data.two[0] = src->device.lightswitch.data.two[0];
		dst->device.lightswitch.data.two[1] = src->device.lightswitch.data.two[1];
		break;
		
	case FRAPP_LIGHTSWITCH_THREE: 
		dst->device.lightswitch.data.three[0] = src->device.lightswitch.data.three[0];
		dst->device.lightswitch.data.three[1] = src->device.lightswitch.data.three[1];
		dst->device.lightswitch.data.three[2] = src->device.lightswitch.data.three[2];
		break;
		
	case FRAPP_LIGHTSWITCH_FOUR: 		
		dst->device.lightswitch.data.four[0] = src->device.lightswitch.data.four[0];
		dst->device.lightswitch.data.four[1] = src->device.lightswitch.data.four[1];
		dst->device.lightswitch.data.four[2] = src->device.lightswitch.data.four[2];
		dst->device.lightswitch.data.four[3] = src->device.lightswitch.data.four[3];
		break;
		
	case FRAPP_ALARM: 
		dst->device.alarm.data[0] = src->device.alarm.data[0];
		break;
		
	case FRAPP_IR_DETECTION: 
		dst->device.irdetect.status[0] = src->device.irdetect.status[0];
		break;
		
	case FRAPP_DOOR_SENSOR: 
		dst->device.doorsensor.status[0] = src->device.doorsensor.status[0];
		break;

	default: return -1;
	}

	return 0;
}


void devopt_de_print(dev_opt_t *opt)
{	
	switch(opt->type)
	{
	case FRAPP_LIGHTSWITCH_ONE: 
		DE_PRINTF("[LightSwitchOne]\n");
		DE_PRINTF("data:%02X\n\n", opt->device.lightswitch.data.one[0]);
		break;
		
	case FRAPP_LIGHTSWITCH_TWO: 
		DE_PRINTF("[LightSwitchTwo]\n");
		DE_PRINTF("data:%02X %02X\n\n", opt->device.lightswitch.data.two[0], 
			opt->device.lightswitch.data.two[1]);
		break;
		
	case FRAPP_LIGHTSWITCH_THREE: 
		DE_PRINTF("[LightSwitchThree]\n");
		DE_PRINTF("data:%02X %02X %02X\n\n", opt->device.lightswitch.data.three[0], 
			opt->device.lightswitch.data.three[1], 
			opt->device.lightswitch.data.three[2]);
		break;
		
	case FRAPP_LIGHTSWITCH_FOUR: 
		DE_PRINTF("[LightSwitchFour]\n");
		DE_PRINTF("data:%02X %02X %02X %02X\n\n", opt->device.lightswitch.data.four[0], 
			opt->device.lightswitch.data.four[1], 
			opt->device.lightswitch.data.four[2], 
			opt->device.lightswitch.data.four[3]);
		break;
		
	case FRAPP_ALARM: 
		DE_PRINTF("[Alarm]\n");
		DE_PRINTF("data:%02X\n\n", opt->device.alarm.data[0]);
		break;
		
	case FRAPP_IR_DETECTION: 
		DE_PRINTF("[IR Detect]\n");
		DE_PRINTF("setting:%d, data:%02X\n\n", opt->device.irdetect.setting, 
			opt->device.irdetect.status[0]);
		break;
		
	case FRAPP_DOOR_SENSOR: 
		DE_PRINTF("[Door Sensor]\n");
		DE_PRINTF("setting:%d, data:%02X\n\n", opt->device.doorsensor.setting, 
			opt->device.doorsensor.status[0]);
		break;
	}
}


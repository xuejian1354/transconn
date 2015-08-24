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

char get_devopt_method_xtoc(control_method_t m);
control_method_t get_devopt_method_ctox(char c);
char get_devopt_power_xtoc(detect_power_t p);
detect_power_t get_devopt_power_ctox(char c);


char get_devopt_method_xtoc(control_method_t m)
{
	switch(m)
	{
	case DEV_CONTROL_MANUAL: return DEVCONTROL_MANUAL;
	case DEV_CONTROL_NET: return DEVCONTROL_NET;
	case DEV_CONTROL_BOTH: return DEVCONTROL_BOTH;
	default: return DEVCONTROL_NONE;
	}
}

control_method_t get_devopt_method_ctox(char c)
{
	switch(c)
	{
	case DEVCONTROL_MANUAL: return DEV_CONTROL_MANUAL;
	case DEVCONTROL_NET: return DEV_CONTROL_NET;
	case DEVCONTROL_BOTH: return DEV_CONTROL_BOTH;
	default: return DEV_CONTROL_NONE;
	}
}

char get_devopt_power_xtoc(detect_power_t p)
{
	switch(p)
	{
	case DEV_DETECT_LOW: return DEVDETECT_LOW;
	case DEV_DETECT_NORMAL: return DEVDETECT_NORMAL;
	}
}

detect_power_t get_devopt_power_ctox(char c)
{
	switch(c)
	{
	case DEVDETECT_LOW: return DEV_DETECT_LOW;
	case DEVDETECT_NORMAL: return DEV_DETECT_NORMAL;
	}
}

dev_opt_t *get_devopt_data_alloc(fr_app_type_t type, uint8 *data, int len)
{
	dev_opt_t *opt = NULL;
	
	switch(type)
	{
	case FRAPP_LIGHTSWITCH_ONE: 
		opt = calloc(1, sizeof(dev_opt_t));
		opt->type = type;
		opt->common.method = DEV_CONTROL_BOTH;
		if(len >= DEVOPT_LIGHTSWITCH_ONE_DATASTR_FIX_SIZE)
		{
			opt->device.lightswitch.nums = 1;
			incode_ctoxs(opt->device.lightswitch.data.one, 
							data, 
							DEVOPT_LIGHTSWITCH_ONE_DATASTR_FIX_SIZE);
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
		if(len >= DEVOPT_LIGHTSWITCH_TWO_DATASTR_FIX_SIZE)
		{
			opt->device.lightswitch.nums = 2;
			incode_ctoxs(opt->device.lightswitch.data.two, 
							data, 
							DEVOPT_LIGHTSWITCH_TWO_DATASTR_FIX_SIZE);
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
		if(len >= DEVOPT_LIGHTSWITCH_THREE_DATASTR_FIX_SIZE)
		{
			opt->device.lightswitch.nums = 3;
			incode_ctoxs(opt->device.lightswitch.data.three, 
							data, 
							DEVOPT_LIGHTSWITCH_THREE_DATASTR_FIX_SIZE);
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
		if(len >= DEVOPT_LIGHTSWITCH_FOUR_DATASTR_FIX_SIZE)
		{
			opt->device.lightswitch.nums = 4;
			incode_ctoxs(opt->device.lightswitch.data.four, 
							data, 
							DEVOPT_LIGHTSWITCH_FOUR_DATASTR_FIX_SIZE);
		}
		else
		{
			opt->device.lightswitch.data.four[0] = 0;
			opt->device.lightswitch.data.four[1] = 0;
			opt->device.lightswitch.data.four[2] = 0;
			opt->device.lightswitch.data.four[3] = 0;
		}
		return opt;

	case FRAPP_HUELIGHT: 
		opt = calloc(1, sizeof(dev_opt_t));
		opt->type = type;
		opt->common.method = DEV_CONTROL_BOTH;
		if(len >= DEVOPT_HUELIGHT_DATASTR_FIX_SIZE)
		{
			incode_ctoxs(&opt->device.huelight, 
							data, 
							DEVOPT_HUELIGHT_DATASTR_FIX_SIZE);
		}
		else
		{
			memset(&opt->device.huelight, 0, 5);
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

	case FRAPP_ENVDETECTION: 
		opt = calloc(1, sizeof(dev_opt_t));
		opt->type = type;
		opt->common.method = DEV_CONTROL_BOTH;

		memset(opt->device.envdetection.current_buffer, 0, 16);
		if(len <= 16)
		{
			memcpy(opt->device.envdetection.current_buffer, data, len);
		}
		
		if(len >= 13 && !memcmp(data, DEVOPT_AIRCONTROLLER_GETDATA, 3))
		{
			incode_ctoxs(&opt->device.envdetection.pm25_thresmode, data+3, 2);
			incode_ctox16(&opt->device.envdetection.pm25_threshold, data+5);
			incode_ctox16(&opt->device.envdetection.pm25_val, data+9);
		}
		else if(len >= 7 && !memcmp(data, DEVOPT_AIRCONTROLLER_PM25READVAL, 3))
		{
			incode_ctox16(&opt->device.envdetection.pm25_val, data+3);
		}
		else if(len >= 5 && !memcmp(data, DEVOPT_AIRCONTROLLER_PM25READMODE, 3))
		{
			incode_ctoxs(&opt->device.envdetection.pm25_thresmode, data+3, 2);
		}
		else if(len >= 7 && !memcmp(data, DEVOPT_AIRCONTROLLER_PM25READHOLD, 3))
		{
			incode_ctox16(&opt->device.envdetection.pm25_threshold, data+3);
		}
		
		return opt;
		
	case FRAPP_IR_RELAY: 
		opt = calloc(1, sizeof(dev_opt_t));
		opt->type = type;
		opt->common.method = DEV_CONTROL_BOTH;
		opt->device.irrelay.mode = 0;
		opt->device.irrelay.data[0] = 0;
		return opt;

	case FRAPP_AIRCONTROLLER:
		opt = calloc(1, sizeof(dev_opt_t));
		opt->type = type;
		opt->common.method = DEV_CONTROL_BOTH;

		memset(opt->device.aircontroller.current_buffer, 0, 16);
		if(len <= 16)
		{
			memcpy(opt->device.aircontroller.current_buffer, data, len);
		}
		
		if(len >= 13 && !memcmp(data, DEVOPT_AIRCONTROLLER_GETDATA, 3))
		{
			incode_ctoxs(&opt->device.aircontroller.pm25_thresmode, data+3, 2);
			incode_ctox16(&opt->device.aircontroller.pm25_threshold, data+5);
			incode_ctox16(&opt->device.aircontroller.pm25_val, data+9);
		}
		else if(len >= 7 && !memcmp(data, DEVOPT_AIRCONTROLLER_PM25READVAL, 3))
		{
			incode_ctox16(&opt->device.aircontroller.pm25_val, data+3);
		}
		else if(len >= 5 && !memcmp(data, DEVOPT_AIRCONTROLLER_PM25READMODE, 3))
		{
			incode_ctoxs(&opt->device.aircontroller.pm25_thresmode, data+3, 2);
		}
		else if(len >= 7 && !memcmp(data, DEVOPT_AIRCONTROLLER_PM25READHOLD, 3))
		{
			incode_ctox16(&opt->device.aircontroller.pm25_threshold, data+3);
		}
		return opt;

	default: return NULL;
	}
}


void get_devopt_data_free(dev_opt_t *opt)
{
	free(opt);
}

int set_devopt_fromstr(dev_opt_t *opt, uint8 *data, int len)
{	
	if(opt == NULL)
	{
		return -1;
	}
	
	switch(opt->type)
	{
	case FRAPP_LIGHTSWITCH_ONE: 
		if(len >= DEVOPT_LIGHTSWITCH_ONE_FIX_SIZE)
		{
			opt->common.method = get_devopt_method_ctox(data[0]);
			incode_ctoxs(&opt->device.lightswitch.nums, data+1, 2);
			incode_ctoxs(opt->device.lightswitch.data.one, data+3, 2);
		}
		else
		{
			return -1;
		}
		break;
		
	case FRAPP_LIGHTSWITCH_TWO: 
		if(len >= DEVOPT_LIGHTSWITCH_TWO_FIX_SIZE)
		{
			opt->common.method = get_devopt_method_ctox(data[0]);
			incode_ctoxs(&opt->device.lightswitch.nums, data+1, 2);
			incode_ctoxs(opt->device.lightswitch.data.two, data+3, 4);
		}
		else
		{
			return -1;
		}
		break;
		
	case FRAPP_LIGHTSWITCH_THREE: 
		if(len >= DEVOPT_LIGHTSWITCH_THREE_FIX_SIZE)
		{
			opt->common.method = get_devopt_method_ctox(data[0]);
			incode_ctoxs(&opt->device.lightswitch.nums, data+1, 2);
			incode_ctoxs(opt->device.lightswitch.data.three, data+3, 6);
		}
		else
		{
			return -1;
		}
		break;
		
	case FRAPP_LIGHTSWITCH_FOUR: 
		if(len >= DEVOPT_LIGHTSWITCH_FOUR_FIX_SIZE)
		{
			opt->common.method = get_devopt_method_ctox(data[0]);
			incode_ctoxs(&opt->device.lightswitch.nums, data+1, 2);
			incode_ctoxs(opt->device.lightswitch.data.four, data+3, 8);
		}
		else
		{
			return -1;
		}
		break;

	case FRAPP_HUELIGHT: 
		if(len >= DEVOPT_HUELIGHT_FIX_SIZE)
		{
			opt->common.method = get_devopt_method_ctox(data[0]);
			incode_ctoxs(&opt->device.huelight, 
							data+1, 
							DEVOPT_HUELIGHT_DATASTR_FIX_SIZE);
		}
		else
		{
			return -1;
		}
		break;
		
	case FRAPP_ALARM: 
		if(len >= DEVOPT_ALARM_FIX_SIZE)
		{
			opt->common.method = get_devopt_method_ctox(data[0]);
			incode_ctoxs(opt->device.alarm.data, data+1, 2);
		}
		else
		{
			return -1;
		}
		break;
		
	case FRAPP_IR_DETECTION: 
		if(len >= DEVOPT_IRDETECT_FIX_SIZE)
		{
			opt->common.power = get_devopt_power_ctox(data[0]);
			incode_ctoxs(&opt->device.irdetect.setting, data+1, 2);
			//incode_ctoxs(opt->device.irdetect.status, data+3, 2);
		}
		else
		{
			return -1;
		}
		break;
		
	case FRAPP_DOOR_SENSOR: 
		if(len >= DEVOPT_DOORSENSOR_FIX_SIZE)
		{
			opt->common.power = get_devopt_power_ctox(data[0]);
			incode_ctoxs(&opt->device.doorsensor.setting, data+1, 2);
			//incode_ctoxs(opt->device.doorsensor.status, data+3, 2);
		}
		else
		{
			return -1;
		}
		break;

	case FRAPP_ENVDETECTION:
		memset(opt->device.envdetection.current_buffer, 0, 16);
		
		if(len <= 16)
		{
			if(!memcmp(data, DEVOPT_AIRCONTROLLER_PM25READVAL, 3)
				|| !memcmp(data, DEVOPT_AIRCONTROLLER_PM25READMODE, 3)
				|| !memcmp(data, DEVOPT_AIRCONTROLLER_PM25READHOLD, 3)
				|| !memcmp(data, DEVOPT_AIRCONTROLLER_GETDATA, 3))
			{
				memcpy(opt->device.envdetection.current_buffer, data, len);
			}			
			else if(len>=5 && !memcmp(data, DEVOPT_AIRCONTROLLER_PM25SETMODE, 3))
			{
				memcpy(opt->device.envdetection.current_buffer, data, len);
				incode_ctoxs(&opt->device.envdetection.pm25_thresmode, data+3, 2);
				if(len >= 9)
				{
					incode_ctox16(&opt->device.envdetection.pm25_threshold, data+5);
				}
			}
			else if(len>=7 &&!memcmp(data, DEVOPT_AIRCONTROLLER_PM25SETHOLD, 3))
			{
				memcpy(opt->device.envdetection.current_buffer, data, len);
				incode_ctox16(&opt->device.envdetection.pm25_threshold, data+3);
			}
			else if(len>=5 && !memcmp(data, DEVOPT_AIRCONTROLLER_UPSETTING, 3))
			{
				memcpy(opt->device.envdetection.current_buffer, data, len);
				incode_ctoxs(&opt->device.envdetection.up_setting, data+3, 2);
			}
		}
		else
		{
			return -1;
		}
		return 0;

	case FRAPP_IR_RELAY:
		if(len >= DEVOPT_IRRELAY_FIX_SIZE)
		{
			if(!memcmp(data, DEVOPT_IRRELAY_LEARN_MODE, 3))
			{
				opt->device.irrelay.mode = 1;
			}
			else if(!memcmp(data, DEVOPT_IRRELAY_SEND_MODE, 3))
			{
				opt->device.irrelay.mode = 0;
			}

			incode_ctoxs(opt->device.irrelay.data, data+3, 2);
		}
		else
		{
			return -1;
		}
		return 0;

	case FRAPP_AIRCONTROLLER:
		memset(opt->device.aircontroller.current_buffer, 0, 16);
		
		if(!memcmp(data, DEVOPT_AIRCONTROLLER_IRSEND, 3))
		{
			uint8 i = 0;
			memcpy(opt->device.aircontroller.current_buffer, DEVOPT_AIRCONTROLLER_IRSEND, 3);
			if(++i && !memcmp(data+3, DEVOPT_AIRCONTROLLER_ON_STR, 2))
			{
				opt->device.aircontroller.aircontrol_onoff = DEVOPT_AIRCONTROLLER_ON;
			}
			else if(++i && !memcmp(data+3, DEVOPT_AIRCONTROLLER_OFF_STR, 3))
			{
				opt->device.aircontroller.aircontrol_onoff = DEVOPT_AIRCONTROLLER_OFF;
			}
			else if(++i && !memcmp(data+3, DEVOPT_AIRCONTROLLER_MODE1_STR, 4))
			{
				opt->device.aircontroller.aircontrol_mode= DEVOPT_AIRCONTROLLER_MODE1;
			}
			else if(++i && !memcmp(data+3, DEVOPT_AIRCONTROLLER_MODE2_STR, 4))
			{
				opt->device.aircontroller.aircontrol_mode = DEVOPT_AIRCONTROLLER_MODE2;
			}
			else if(++i && !memcmp(data+3, DEVOPT_AIRCONTROLLER_MODE3_STR, 4))
			{
				opt->device.aircontroller.aircontrol_mode = DEVOPT_AIRCONTROLLER_MODE3;
			}
			else
			{
				++i;
			}

			if(i < 6)
			{
				incode_xtocs(opt->device.aircontroller.current_buffer+3, &i, 1);
			}
		}
		else if(!memcmp(data, DEVOPT_AIRCONTROLLER_IRLEARN, 3))
		{
			uint8 i = 0;
			memcpy(opt->device.aircontroller.current_buffer, DEVOPT_AIRCONTROLLER_IRLEARN, 3);
			if(++i && memcmp(data+3, DEVOPT_AIRCONTROLLER_ON_STR, 2)
				&& ++i && memcmp(data+3, DEVOPT_AIRCONTROLLER_OFF_STR, 3)
					&& ++i && memcmp(data+3, DEVOPT_AIRCONTROLLER_MODE1_STR, 4)
				 		&& ++i && memcmp(data+3, DEVOPT_AIRCONTROLLER_MODE2_STR, 4)
				 			&& ++i && memcmp(data+3, DEVOPT_AIRCONTROLLER_MODE3_STR, 4)
				 				&& ++i) {}

			if(i < 6)
			{
				incode_xtocs(opt->device.aircontroller.current_buffer+3, &i, 1);
			}
		}
		else if(len <= 16)
		{
			if(!memcmp(data, DEVOPT_AIRCONTROLLER_PM25READVAL, 3)
				|| !memcmp(data, DEVOPT_AIRCONTROLLER_PM25READMODE, 3)
				|| !memcmp(data, DEVOPT_AIRCONTROLLER_PM25READHOLD, 3)
				|| !memcmp(data, DEVOPT_AIRCONTROLLER_GETDATA, 3))
			{
				memcpy(opt->device.aircontroller.current_buffer, data, len);
			}			
			else if(len>=5 && !memcmp(data, DEVOPT_AIRCONTROLLER_PM25SETMODE, 3))
			{
				memcpy(opt->device.aircontroller.current_buffer, data, len);
				incode_ctoxs(&opt->device.aircontroller.pm25_thresmode, data+3, 2);
				if(len >= 9)
				{
					incode_ctox16(&opt->device.aircontroller.pm25_threshold, data+5);
				}
			}
			else if(len>=7 &&!memcmp(data, DEVOPT_AIRCONTROLLER_PM25SETHOLD, 3))
			{
				memcpy(opt->device.aircontroller.current_buffer, data, len);
				incode_ctox16(&opt->device.aircontroller.pm25_threshold, data+3);
			}
			else if(len>=5 && !memcmp(data, DEVOPT_AIRCONTROLLER_UPSETTING, 3))
			{
				memcpy(opt->device.aircontroller.current_buffer, data, len);
				incode_ctoxs(&opt->device.aircontroller.up_setting, data+3, 2);
			}
		}
		else
		{
			return -1;
		}
		return 0;

	default: return -1;
	}

	return 0;
}

fr_buffer_t *get_devopt_buffer_alloc(dev_opt_t *opt, uint8 *data, uint8 datalen)
{
	fr_buffer_t *buffer = NULL;

	if(opt == NULL)
	{
		return NULL;
	}
	
	switch(opt->type)
	{
	case FRAPP_LIGHTSWITCH_ONE: 
		buffer = calloc(1, sizeof(fr_buffer_t));
		buffer->size = DEVOPT_LIGHTSWITCH_ONE_FIX_SIZE;
		buffer->data = calloc(1, buffer->size);
		
		buffer->data[0] = get_devopt_method_xtoc(opt->common.method);
		incode_xtocs(buffer->data+1, &opt->device.lightswitch.nums, 1);
		incode_xtocs(buffer->data+3, opt->device.lightswitch.data.one, 1);
		return buffer;
		
	case FRAPP_LIGHTSWITCH_TWO: 
		buffer = calloc(1, sizeof(fr_buffer_t));
		buffer->size = DEVOPT_LIGHTSWITCH_TWO_FIX_SIZE;
		buffer->data = calloc(1, buffer->size);
		
		buffer->data[0] = get_devopt_method_xtoc(opt->common.method);
		incode_xtocs(buffer->data+1, &opt->device.lightswitch.nums, 1);
		incode_xtocs(buffer->data+3, opt->device.lightswitch.data.two, 2);
		return buffer;
		
	case FRAPP_LIGHTSWITCH_THREE: 
		buffer = calloc(1, sizeof(fr_buffer_t));
		buffer->size = DEVOPT_LIGHTSWITCH_THREE_FIX_SIZE;
		buffer->data = calloc(1, buffer->size);
		
		buffer->data[0] = get_devopt_method_xtoc(opt->common.method);
		incode_xtocs(buffer->data+1, &opt->device.lightswitch.nums, 1);
		incode_xtocs(buffer->data+3, opt->device.lightswitch.data.three, 3);
		return buffer;
		
	case FRAPP_LIGHTSWITCH_FOUR: 
		buffer = calloc(1, sizeof(fr_buffer_t));
		buffer->size = DEVOPT_LIGHTSWITCH_FOUR_FIX_SIZE;
		buffer->data = calloc(1, buffer->size);
		
		buffer->data[0] = get_devopt_method_xtoc(opt->common.method);
		incode_xtocs(buffer->data+1, &opt->device.lightswitch.nums, 1);
		incode_xtocs(buffer->data+3, opt->device.lightswitch.data.four, 4);
		return buffer;

	case FRAPP_HUELIGHT: 
		buffer = calloc(1, sizeof(fr_buffer_t));
		buffer->size = DEVOPT_HUELIGHT_FIX_SIZE;
		buffer->data = calloc(1, buffer->size);
		
		buffer->data[0] = get_devopt_method_xtoc(opt->common.method);
		incode_xtocs(buffer->data+1, &opt->device.huelight, 4);
		return buffer;
		
	case FRAPP_ALARM: 
		buffer = calloc(1, sizeof(fr_buffer_t));
		buffer->size = DEVOPT_ALARM_FIX_SIZE;
		buffer->data = calloc(1, buffer->size);
		
		buffer->data[0] = get_devopt_method_xtoc(opt->common.method);
		incode_xtocs(buffer->data+1, opt->device.alarm.data, 1);
		return buffer;
		
	case FRAPP_IR_DETECTION: 
		buffer = calloc(1, sizeof(fr_buffer_t));
		buffer->size = DEVOPT_IRDETECT_FIX_SIZE;
		buffer->data = calloc(1, buffer->size);
		
		buffer->data[0] = get_devopt_power_xtoc(opt->common.power);
		incode_xtocs(buffer->data+1, &opt->device.irdetect.setting, 1);
		incode_xtocs(buffer->data+3, opt->device.irdetect.status, 1);
		return buffer;
		
	case FRAPP_DOOR_SENSOR: 
		buffer = calloc(1, sizeof(fr_buffer_t));
		buffer->size = DEVOPT_DOORSENSOR_FIX_SIZE;
		buffer->data = calloc(1, buffer->size);
		
		buffer->data[0] = get_devopt_power_xtoc(opt->common.power);
		incode_xtocs(buffer->data+1, &opt->device.doorsensor.setting, 1);
		incode_xtocs(buffer->data+3, opt->device.doorsensor.status, 1);
		return buffer;

	case FRAPP_ENVDETECTION:
		if(datalen > 16)
		{
			return NULL;
		}
		
		memset(opt->device.envdetection.current_buffer, 0, 16);
		if(datalen >= 13 && !memcmp(data, DEVOPT_AIRCONTROLLER_GETDATA, 3))
		{
			memcpy(opt->device.envdetection.current_buffer, data, datalen);
			incode_ctoxs(&opt->device.envdetection.pm25_thresmode, data+3, 2);
			incode_ctox16(&opt->device.envdetection.pm25_threshold, data+5);
			incode_ctox16(&opt->device.envdetection.pm25_val, data+9);
		}
		else if(datalen >= 7 && !memcmp(data, DEVOPT_AIRCONTROLLER_PM25READVAL, 3))
		{
			memcpy(opt->device.envdetection.current_buffer, data, datalen);
			incode_ctox16(&opt->device.envdetection.pm25_val, data+3);
		}
		else if(datalen >= 5 && !memcmp(data, DEVOPT_AIRCONTROLLER_PM25READMODE, 3))
		{
			memcpy(opt->device.envdetection.current_buffer, data, datalen);
			incode_ctoxs(&opt->device.envdetection.pm25_thresmode, data+3, 2);
		}
		else if(datalen >= 7 && !memcmp(data, DEVOPT_AIRCONTROLLER_PM25READHOLD, 3))
		{
			memcpy(opt->device.envdetection.current_buffer, data, datalen);
			incode_ctox16(&opt->device.envdetection.pm25_threshold, data+3);
		}
		else if(datalen >= 5 && !memcmp(data, DEVOPT_AIRCONTROLLER_PM25SETMODE, 3))
		{
			memcpy(opt->device.envdetection.current_buffer, data, datalen);
		}
		else if(datalen >= 7 && !memcmp(data, DEVOPT_AIRCONTROLLER_PM25SETHOLD, 3))
		{
			memcpy(opt->device.envdetection.current_buffer, data, datalen);
		}
		else if(datalen >= 5 && !memcmp(data, DEVOPT_AIRCONTROLLER_UPSETTING, 3))
		{
			memcpy(opt->device.envdetection.current_buffer, 
									DEVOPT_AIRCONTROLLER_UPSETTING, 3);
			memcpy(opt->device.envdetection.current_buffer+3, "OK\0", 3);
		}
		else
		{
			memcpy(opt->device.envdetection.current_buffer, data, datalen);
		}
		
		buffer = calloc(1, sizeof(fr_buffer_t));
		buffer->size = strlen(opt->device.envdetection.current_buffer);
		buffer->data = calloc(1, buffer->size);
		memcpy(buffer->data, 
			opt->device.envdetection.current_buffer, buffer->size);
		return buffer;	
		
	case FRAPP_IR_RELAY: 
		return NULL;

	case FRAPP_AIRCONTROLLER:
		if(datalen > 16)
		{
			return NULL;
		}
		
		memset(opt->device.envdetection.current_buffer, 0, 16);
		
		if(datalen >= 13 && !memcmp(data, DEVOPT_AIRCONTROLLER_GETDATA, 3))
		{
			memcpy(opt->device.aircontroller.current_buffer, data, datalen);
			incode_ctoxs(&opt->device.aircontroller.pm25_thresmode, data+3, 2);
			incode_ctox16(&opt->device.aircontroller.pm25_threshold, data+5);
			incode_ctox16(&opt->device.aircontroller.pm25_val, data+9);
		}
		else if(datalen >= 7 && !memcmp(data, DEVOPT_AIRCONTROLLER_PM25READVAL, 3))
		{
			memcpy(opt->device.aircontroller.current_buffer, data, datalen);
			incode_ctox16(&opt->device.aircontroller.pm25_val, data+3);
		}
		else if(datalen >= 5 && !memcmp(data, DEVOPT_AIRCONTROLLER_PM25READMODE, 3))
		{
			memcpy(opt->device.aircontroller.current_buffer, data, datalen);
			incode_ctoxs(&opt->device.aircontroller.pm25_thresmode, data+3, 2);
		}
		else if(datalen >= 7 && !memcmp(data, DEVOPT_AIRCONTROLLER_PM25READHOLD, 3))
		{
			memcpy(opt->device.aircontroller.current_buffer, data, datalen);
			incode_ctox16(&opt->device.aircontroller.pm25_threshold, data+3);
		}
		else if(datalen >= 5 && !memcmp(data, DEVOPT_AIRCONTROLLER_PM25SETMODE, 3))
		{
			memcpy(opt->device.aircontroller.current_buffer, data, datalen);
		}
		else if(datalen >= 7 && !memcmp(data, DEVOPT_AIRCONTROLLER_PM25SETHOLD, 3))
		{
			memcpy(opt->device.aircontroller.current_buffer, data, datalen);
		}
		else if(datalen >= 5 && !memcmp(data, DEVOPT_AIRCONTROLLER_UPSETTING, 3))
		{
			memcpy(opt->device.aircontroller.current_buffer, 
									DEVOPT_AIRCONTROLLER_UPSETTING, 3);
			memcpy(opt->device.aircontroller.current_buffer+3, "OK\0", 3);
		}
		else
		{
			memcpy(opt->device.aircontroller.current_buffer, data, datalen);
		}
		
		buffer = calloc(1, sizeof(fr_buffer_t));
		buffer->size = strlen(opt->device.aircontroller.current_buffer);
		buffer->data = calloc(1, buffer->size);
		memcpy(buffer->data, 
			opt->device.aircontroller.current_buffer, buffer->size);
		return buffer;		

	default: return NULL;
	}
}


void *get_devopt_buffer_free(fr_buffer_t *p)
{
	if(p != NULL)
	{
		free(p->data);
	}
	
	free(p);
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

	case FRAPP_HUELIGHT: 
		if(len >= 5)
		{
			memcpy(&opt->device.huelight, data, 5);
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


fr_buffer_t * get_devopt_data_to_str(dev_opt_t *opt)
{	
	fr_buffer_t *buffer = NULL;
	if(opt == NULL)
	{
		return NULL;
	}
	
	switch(opt->type)
	{
	case FRAPP_LIGHTSWITCH_ONE: 
		buffer = calloc(1, sizeof(fr_buffer_t));
		buffer->size = DEVOPT_LIGHTSWITCH_ONE_DATASTR_FIX_SIZE;
		buffer->data = calloc(1, buffer->size);
		incode_xtocs(buffer->data, opt->device.lightswitch.data.one, 1);
		return buffer;
		
	case FRAPP_LIGHTSWITCH_TWO: 
		buffer = calloc(1, sizeof(fr_buffer_t));
		buffer->size = DEVOPT_LIGHTSWITCH_TWO_DATASTR_FIX_SIZE;
		buffer->data = calloc(1, buffer->size);
		incode_xtocs(buffer->data, opt->device.lightswitch.data.two, 2);
		return buffer;
		
	case FRAPP_LIGHTSWITCH_THREE: 
		buffer = calloc(1, sizeof(fr_buffer_t));
		buffer->size = DEVOPT_LIGHTSWITCH_THREE_DATASTR_FIX_SIZE;
		buffer->data = calloc(1, buffer->size);
		incode_xtocs(buffer->data, opt->device.lightswitch.data.three, 3);
		return buffer;
		
	case FRAPP_LIGHTSWITCH_FOUR: 
		buffer = calloc(1, sizeof(fr_buffer_t));
		buffer->size = DEVOPT_LIGHTSWITCH_FOUR_DATASTR_FIX_SIZE;
		buffer->data = calloc(1, buffer->size);
		incode_xtocs(buffer->data, opt->device.lightswitch.data.four, 4);
		return buffer;

	case FRAPP_HUELIGHT: 
		buffer = calloc(1, sizeof(fr_buffer_t));
		buffer->size = DEVOPT_HUELIGHT_DATASTR_FIX_SIZE;
		buffer->data = calloc(1, buffer->size);
		incode_xtocs(buffer->data, opt->device.huelight, 5);
		return buffer;
		
	case FRAPP_ALARM: 
		buffer = calloc(1, sizeof(fr_buffer_t));
		buffer->size = DEVOPT_ALARM_DATASTR_FIX_SIZE;
		buffer->data = calloc(1, buffer->size);
		incode_xtocs(buffer->data, opt->device.alarm.data, 1);
		return buffer;
		
	case FRAPP_IR_DETECTION: 
		buffer = calloc(1, sizeof(fr_buffer_t));
		buffer->size = DEVOPT_IRDETECT_DATASTR_FIX_SIZE;
		buffer->data = calloc(1, buffer->size);
		incode_xtocs(buffer->data, &opt->device.irdetect.setting, 1);
		return buffer;
		
	case FRAPP_DOOR_SENSOR: 
		buffer = calloc(1, sizeof(fr_buffer_t));
		buffer->size = DEVOPT_DOORSENSOR_DATASTR_FIX_SIZE;
		buffer->data = calloc(1, buffer->size);
		incode_xtocs(buffer->data, &opt->device.doorsensor.setting, 1);
		return buffer;

	case FRAPP_ENVDETECTION:
		buffer = calloc(1, sizeof(fr_buffer_t));
		buffer->size = strlen(opt->device.envdetection.current_buffer);
		buffer->data = calloc(1, buffer->size);
		memcpy(buffer->data, 
			opt->device.envdetection.current_buffer, buffer->size);
		return buffer;

	case FRAPP_IR_RELAY:
		buffer = calloc(1, sizeof(fr_buffer_t));
		buffer->size = DEVOPT_IRRELAY_DATASTR_FIX_SIZE;
		buffer->data = calloc(1, buffer->size);
		if(opt->device.irrelay.mode == 0)
		{
			memcpy(buffer->data, DEVOPT_IRRELAY_SEND_MODE, 3);
		}
		else if(opt->device.irrelay.mode == 1)
		{
			memcpy(buffer->data, DEVOPT_IRRELAY_LEARN_MODE, 3);
		}

		incode_xtocs(buffer->data+3, opt->device.irrelay.data, 1);
		
		return buffer;

	case FRAPP_AIRCONTROLLER:
		buffer = calloc(1, sizeof(fr_buffer_t));
		buffer->size = strlen(opt->device.aircontroller.current_buffer);
		buffer->data = calloc(1, buffer->size);
		memcpy(buffer->data, 
			opt->device.aircontroller.current_buffer, buffer->size);
		return buffer;

	default: return NULL;
	}
}


int set_devopt_data_fromopt(dev_opt_t *dst, dev_opt_t *src)
{	
	if(dst == NULL || src == NULL || dst->type != src->type)
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

	case FRAPP_HUELIGHT: 		
		memcpy(&dst->device.huelight, &src->device.huelight, 5);
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

	case FRAPP_ENVDETECTION:
		if(!memcmp(src->device.envdetection.current_buffer, 
									DEVOPT_AIRCONTROLLER_GETDATA, 3))
		{
			dst->device.envdetection.pm25_thresmode = 
						src->device.envdetection.pm25_thresmode;
			dst->device.envdetection.pm25_threshold = 
						src->device.envdetection.pm25_threshold;
			dst->device.envdetection.pm25_val = 
						src->device.envdetection.pm25_val;
		}
		else if(!memcmp(src->device.envdetection.current_buffer, 
								DEVOPT_AIRCONTROLLER_PM25READVAL, 3))
		{
			dst->device.envdetection.pm25_val = 
						src->device.envdetection.pm25_val;
		}
		else if(!memcmp(src->device.envdetection.current_buffer, 
								DEVOPT_AIRCONTROLLER_PM25READMODE, 3))
		{
			dst->device.envdetection.pm25_thresmode = 
						src->device.envdetection.pm25_thresmode;
		}
		else if(!memcmp(src->device.envdetection.current_buffer, 
								DEVOPT_AIRCONTROLLER_PM25READHOLD, 3))
		{
			dst->device.envdetection.pm25_threshold = 
						src->device.envdetection.pm25_threshold;
		}
		break;

	case FRAPP_AIRCONTROLLER:
		if(!memcmp(src->device.aircontroller.current_buffer, 
									DEVOPT_AIRCONTROLLER_GETDATA, 3))
		{
			dst->device.aircontroller.pm25_thresmode = 
						src->device.aircontroller.pm25_thresmode;
			dst->device.aircontroller.pm25_threshold = 
						src->device.aircontroller.pm25_threshold;
			dst->device.aircontroller.pm25_val = 
						src->device.aircontroller.pm25_val;
		}
		else if(!memcmp(src->device.aircontroller.current_buffer, 
								DEVOPT_AIRCONTROLLER_PM25READVAL, 3))
		{
			dst->device.aircontroller.pm25_val = 
						src->device.aircontroller.pm25_val;
		}
		else if(!memcmp(src->device.aircontroller.current_buffer, 
								DEVOPT_AIRCONTROLLER_PM25READMODE, 3))
		{
			dst->device.aircontroller.pm25_thresmode = 
						src->device.aircontroller.pm25_thresmode;
		}
		else if(!memcmp(src->device.aircontroller.current_buffer, 
								DEVOPT_AIRCONTROLLER_PM25READHOLD, 3))
		{
			dst->device.aircontroller.pm25_threshold = 
						src->device.aircontroller.pm25_threshold;
		}
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

	case FRAPP_HUELIGHT: 
		DE_PRINTF("[HueLight]\n");
		DE_PRINTF("data:%02X %02X %02X %02X %02X\n\n", 
			opt->device.huelight.bright, 
			opt->device.huelight.saturation, 
			opt->device.huelight.red, 
			opt->device.huelight.green,
			opt->device.huelight.blue);
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

	case FRAPP_ENVDETECTION:
		DE_PRINTF("[Env Detection]\n");
		DE_PRINTF("thresmode:%d, threshold:%d, val:%d\n\n", 
			opt->device.envdetection.pm25_thresmode, 
			opt->device.envdetection.pm25_threshold,
			opt->device.envdetection.pm25_val);
		break;

	case FRAPP_AIRCONTROLLER:
		DE_PRINTF("[Air Controller]\n");
		DE_PRINTF("thresmode:%d, threshold:%d, val:%d\n\n", 
			opt->device.aircontroller.pm25_thresmode, 
			opt->device.aircontroller.pm25_threshold,
			opt->device.aircontroller.pm25_val);
		break;
	}
}


/*
 * devopt.h
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
#ifndef __DEVOPT_H__
#define __DEVOPT_H__

#include <services/globals.h>
#include <protocol/framelysis.h>

#define DEVCONTROL_NONE		'0'
#define DEVCONTROL_MANUAL	'1'
#define DEVCONTROL_NET		'2'
#define DEVCONTROL_BOTH		'3'

#define DEVDETECT_LOW		'1'
#define DEVDETECT_NORMAL	'2'

#define DEVOPT_LIGHTSWITCH_ONE_FIX_SIZE		5
#define DEVOPT_LIGHTSWITCH_TWO_FIX_SIZE		7
#define DEVOPT_LIGHTSWITCH_THREE_FIX_SIZE	9
#define DEVOPT_LIGHTSWITCH_FOUR_FIX_SIZE	11
#define DEVOPT_HUELIGHT_FIX_SIZE			9
#define DEVOPT_ALARM_FIX_SIZE				3
#define DEVOPT_IRRELAY_FIX_SIZE				5
#define DEVOPT_IRDETECT_FIX_SIZE			5
#define DEVOPT_DOORSENSOR_FIX_SIZE			5
#define DEVOPT_RELAYSOCKET_FIX_SIZE			3
#define DEVOPT_LIGHTDETECT_FIX_SIZE			5

#define DEVOPT_LIGHTSWITCH_ONE_DATASTR_FIX_SIZE		2
#define DEVOPT_LIGHTSWITCH_TWO_DATASTR_FIX_SIZE		4
#define DEVOPT_LIGHTSWITCH_THREE_DATASTR_FIX_SIZE	6
#define DEVOPT_LIGHTSWITCH_FOUR_DATASTR_FIX_SIZE	8
#define DEVOPT_HUELIGHT_DATASTR_FIX_SIZE			8
#define DEVOPT_ALARM_DATASTR_FIX_SIZE				2
#define DEVOPT_IRRELAY_DATASTR_FIX_SIZE				5
#define DEVOPT_IRDETECT_DATASTR_FIX_SIZE			2
#define DEVOPT_DOORSENSOR_DATASTR_FIX_SIZE			2
#define DEVOPT_RELAYSOCKET_DATASTR_FIX_SIZE			2
#define DEVOPT_LIGHTDETECT_DATASTR_FIX_SIZE			4

#define DEVOPT_IRRELAY_LEARN_MODE	"LEA"
#define DEVOPT_IRRELAY_SEND_MODE	"SEN"


#define DEVOPT_AIRCONTROLLER_IRSEND			"SEN"
#define DEVOPT_AIRCONTROLLER_IRLEARN		"LEA"
#define DEVOPT_AIRCONTROLLER_PM25READVAL	"REV"
#define DEVOPT_AIRCONTROLLER_PM25READMODE	"REM"
#define DEVOPT_AIRCONTROLLER_PM25READHOLD	"REH"
#define DEVOPT_AIRCONTROLLER_PM25SETMODE	"WRM"
#define DEVOPT_AIRCONTROLLER_PM25SETHOLD	"WRH"
#define DEVOPT_AIRCONTROLLER_GETDATA		"GDT"
#define DEVOPT_AIRCONTROLLER_UPSETTING		"UPS"

#define DEVOPT_AIRCONTROLLER_ON_STR			"ON"
#define DEVOPT_AIRCONTROLLER_OFF_STR		"OFF"
#define DEVOPT_AIRCONTROLLER_MODE1_STR		"MOD1"
#define DEVOPT_AIRCONTROLLER_MODE2_STR		"MOD2"
#define DEVOPT_AIRCONTROLLER_MODE3_STR		"MOD3"

#define DEVOPT_AIRCONTROLLER_OK_REG		"OK"
#define DEVOPT_AIRCONTROLLER_NO_REG		"NO"

#define DEVOPT_AIRCONTROLLER_ON				1
#define DEVOPT_AIRCONTROLLER_OFF			0
#define DEVOPT_AIRCONTROLLER_MODE1			1
#define DEVOPT_AIRCONTROLLER_MODE2			2
#define DEVOPT_AIRCONTROLLER_MODE3			3

typedef enum
{
	DEV_CONTROL_MANUAL,
	DEV_CONTROL_NET,
	DEV_CONTROL_BOTH,
	DEV_CONTROL_NONE,
}control_method_t;

typedef enum
{
	DEV_DETECT_LOW,
	DEV_DETECT_NORMAL,
}detect_power_t;

typedef struct Switch_Opt
{
	uint8 nums;
	union
	{
		uint8 one[1];
		uint8 two[2];
		uint8 three[3];
		uint8 four[4];
	}data;
}switch_opt_t;

typedef struct HueLight_Opt
{
	uint8 onoff;
	uint8 bright;
	uint8 hue;
	uint8 saturation;
	cidentify_no_t sclient;
}huelight_opt_t;

typedef struct Alarm_Opt
{
	uint8 data[1];
}alarm_opt_t;

typedef struct IRRelay_Opt
{
	uint8 mode;		//0 send, 1 learn
	uint8 data[1];
}irrelay_opt_t;

typedef struct IRDetect_Opt
{
	uint8 setting;
	uint8 status[1];
}irdetect_opt_t;

typedef struct DoorSensor_Opt
{
	uint8 setting;
	uint8 status[1];
}doorsensor_opt_t;

typedef struct EnvDetection_Opt
{
	uint8 up_setting;
	uint8 current_buffer[16];
	uint8 pm25_thresmode;
	uint16 pm25_threshold;
	uint16 pm25_val;
}envdetection_opt_t;

typedef struct AirController_Opt
{
	uint8 up_setting;
	uint8 current_buffer[16];
	uint8 pm25_thresmode;
	uint16 pm25_threshold;
	uint16 pm25_val;
	uint8 aircontrol_onoff;
	uint8 aircontrol_mode;
}aircontroller_opt_t;

typedef struct RelaySocket_Opt
{
	uint8 data[1];
}relaysocket_opt_t;

typedef struct LightDetect_Opt
{
	uint16 data;
}lightdetect_opt_t;

typedef struct Dev_Opt
{
	fr_app_type_t type;
	
	union
	{
		detect_power_t power;
		control_method_t method;
	}common;

	union
	{
		switch_opt_t lightswitch;
		huelight_opt_t huelight;
		alarm_opt_t alarm;
		irrelay_opt_t irrelay;
		irdetect_opt_t irdetect;
		doorsensor_opt_t doorsensor;
		envdetection_opt_t envdetection;
		aircontroller_opt_t aircontroller;
		relaysocket_opt_t relaysocket;
		lightdetect_opt_t lightdetect;
	}device;
}dev_opt_t;

fr_buffer_t *get_devopt_buffer_alloc(dev_opt_t *opt, uint8 *data, uint8 datalen);
void *get_devopt_buffer_free(fr_buffer_t *p);

dev_opt_t *get_devopt_data_alloc(fr_app_type_t type, uint8 *data, int len);
void get_devopt_data_free(dev_opt_t *opt);

int set_devopt_fromstr(dev_opt_t *opt, uint8 *data, int len);
fr_buffer_t * get_devopt_data_to_str(dev_opt_t *opt);

int set_devopt_data(dev_opt_t *opt, uint8 *data, int len);
int set_devopt_data_fromopt(dev_opt_t *dst, dev_opt_t *src);

void devopt_de_print(dev_opt_t *opt);

#endif	// __DEVOPT_H__

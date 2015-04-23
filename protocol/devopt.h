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
	int nums;
	union
	{
		int one;
		int two[2];
		int three[3];
	}data;
}switch_opt_t;

typedef struct Alarm_Opt
{
	int data;
}alarm_opt_t;

typedef struct IRRelay_Opt
{
	int mode;
}irrelay_opt_t;

typedef struct IRDetect_Opt
{
	int setting;
	int status;
}irdetect_opt_t;

typedef struct DoorSensor_Opt
{
	int setting;
	int status;
}doorsensor_opt_t;


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
		alarm_opt_t alarm;
		irrelay_opt_t irrelay;
		irdetect_opt_t irdetect;
		doorsensor_opt_t doorsensor;
	}device;
}dev_opt_t;

#endif	// __DEVOPT_H__
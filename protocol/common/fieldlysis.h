/*
 * fieldlysis.h
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
#ifndef __FIELDLYSIS_H__
#define __FIELDLYSIS_H__

#include <services/globals.h>

#ifdef __cplusplus
extern "C" {
#endif

#define JSON_FIELD_NAME_MAXSIZE			64
#define JSON_FIELD_OBJ_MAXSIZE			32
#define JSON_FIELD_SN_MAXSIZE			24
#define JSON_FIELD_DATA_MAXSIZE			32
#define JSON_FIELD_CMD_MAXSIZE			32
#define JSON_FIELD_TOCOL_MAXSIZE		12
#define JSON_FIELD_CODECHECK_MAXSIZE	8
#define JSON_FIELD_CODEDATA_MAXSIZE		40
#define JSON_FIELD_INFO_MAXSIZE			8
#define JSON_FIELD_RANDOM_MAXSIZE		6

#define JSON_FIELD_ACTION		"action"
#define JSON_FIELD_OBJECT		"obj"
#define JSON_FIELD_OWNER		"owner"
#define JSON_FIELD_CUSTOM		"custom"
#define JSON_FIELD_REQACTION	"req_action"
#define JSON_FIELD_PROTOCOL		"protocol"
#define JSON_FIELD_GWSN			"gw_sn"
#define JSON_FIELD_DEVICES		"devices"
#define JSON_FIELD_NAME			"name"
#define JSON_FIELD_DEVSN		"dev_sn"
#define JSON_FIELD_DEVSNS		"dev_sns"
#define JSON_FIELD_DEVTYPE		"dev_type"
#define JSON_FIELD_DEVDATA		"dev_data"
#define JSON_FIELD_ZSTATUS		"znet_status"
#define JSON_FIELD_CODE			"code"
#define JSON_FIELD_CODECHECK	"code_check"
#define JSON_FIELD_CODEDATA		"code_data"
#define JSON_FIELD_CTRLS		"ctrls"
#define JSON_FIELD_CMD			"cmd"
#define JSON_FIELD_INFO			"info"
#define JSON_FIELD_RANDOM		"random"

#define ZH_TYPE_NAME

#ifdef ZH_TYPE_NAME
#define NO_AREA		"未设置"
#else
#define NO_AREA		"NotSetting"
#endif

#define INFO_TIMEOUT	"timeout"

#define INFO_OBJECT_GATEWAY		"gateway"
#define INFO_OBJECT_DEVICE		"device"
#define INFO_OBJECT_SERVER		"server"
#define INFO_OBJECT_CLIENT		"client"
#define INFO_OBJECT_DEBUG		"debug"


typedef enum
{
	FRAPP_CONNECTOR = 0,
	FRAPP_CAMERA = 1,
	FRAPP_TEMP = 2,
	FRAPP_HUMI = 3,
	FRAPP_ILLUMI = 4,
	FRAPP_RAINFALL = 5,
	FRAPP_AIRSPEED = 6,
	FRAPP_AIRDIRECTION = 7,
	FRAPP_AMMONIA = 8,
	FRAPP_NITROGEN = 9,
	FRAPP_HYDROTHION = 10,
	FRAPP_DUST = 11,
	FRAPP_NOISE = 12,
	FRAPP_PRESSURE = 13,
	FRAPP_DIOXIDE = 14,
	FRAPP_WATER_DIOXIDE = 15,
	FRAPP_WATER_NITROGEN = 16,
	FRAPP_WATER_NITRITE = 17,
	FRAPP_WATER_TEMP = 18,
	FRAPP_WATER_PH = 19,
	FRAPP_SOIL_TEMP = 20,
	FRAPP_SOIL_MOISTURE = 21,
	FRAPP_SOIL_SALINITY = 22,
	FRAPP_SOIL_PH = 23,
	FRAPP_FERTILIZER_CTRL = 24,
	FRAPP_AERATOR_CTRL = 25,
	FRAPP_ELECTRIC_CTRL = 26,
	FRAPP_FAN_CTRL = 27,
	FRAPP_PUMP_CTRL = 28,
	FRAPP_VALVE_CTRL = 29,
	FRAPP_SOLENOID_CTRL = 30,
	FRAPP_SHUTTER_CTRL = 31,

}fr_app_type_t;

typedef enum
{
	TN_CONNECTOR,
	TN_CAMERA,
	TN_TEMP,
	TN_HUMI,
	TN_ILLUMI,
	TN_RAINFALL,
	TN_AIRSPEED,
	TN_AIRDIRECTION,
	TN_AMMONIA,
	TN_NITROGEN,
	TN_HYDROTHION,
	TN_DUST,
	TN_NOISE,
	TN_PRESSURE,
	TN_DIOXIDE,
	TN_WATER_DIOXIDE,
	TN_WATER_NITROGEN,
	TN_WATER_NITRITE,
	TN_WATER_TEMP,
	TN_WATER_PH,
	TN_SOIL_TEMP,
	TN_SOIL_MOISTURE,
	TN_SOIL_SALINITY,
	TN_SOIL_PH,
	TN_FERTILIZER_CTRL,
	TN_AERATOR_CTR,
	TN_ELECTRIC_CTRL,
	TN_FAN_CTRL,
	TN_PUMP_CTRL,
	TN_VALVE_CTRL,
	TN_SOLENOID_CTRL,
	TN_SHUTTER_CTRL,
	TN_NUMS
}type_name_t;

typedef struct
{
    fr_app_type_t type;
    const char *val;
}zh_el_t;

typedef enum
{
	ACTION_TOCOLREQ = 1,
	ACTION_REPORT,
	ACTION_CHECK,
	ACTION_RESPOND,
	ACTION_REFRESH,
	ACTION_CONTROL,
	ACTION_TOCOLRES
}trans_action_t;

typedef char sn_t[JSON_FIELD_SN_MAXSIZE];

typedef struct
{
	char name[JSON_FIELD_NAME_MAXSIZE];
	sn_t dev_sn;
	fr_app_type_t dev_type;
	int znet_status;
	char dev_data[JSON_FIELD_DATA_MAXSIZE];
}trfield_device_t;

typedef struct
{
	sn_t dev_sn;
	char cmd[JSON_FIELD_CMD_MAXSIZE];
}trfield_ctrl_t;

typedef struct
{
	char owner[JSON_FIELD_OBJ_MAXSIZE];
	char custom[JSON_FIELD_OBJ_MAXSIZE];
}trfield_obj_t;

typedef struct
{
	trans_action_t action;
	trfield_obj_t *obj;
	sn_t gw_sn;
	char protocol[JSON_FIELD_TOCOL_MAXSIZE];
	char random[JSON_FIELD_RANDOM_MAXSIZE];
}trfr_tocolreq_t;

typedef struct
{
	trans_action_t action;
	trfield_obj_t *obj;
	sn_t gw_sn;
	trfield_device_t **devices;
	int dev_size;
	char random[JSON_FIELD_RANDOM_MAXSIZE];
}trfr_report_t;

typedef struct
{
	trans_action_t action;
	trfield_obj_t *obj;
	sn_t gw_sn;
	sn_t *dev_sns;
	int sn_size;
	struct {
		char code_check[JSON_FIELD_CODECHECK_MAXSIZE];
		char code_data[JSON_FIELD_CODEDATA_MAXSIZE];
	} code;
	char random[JSON_FIELD_RANDOM_MAXSIZE];
}trfr_check_t;

typedef struct
{
	trans_action_t action;
	trfield_obj_t *obj;
	sn_t gw_sn;
	sn_t dev_sn;
	char dev_data[JSON_FIELD_DATA_MAXSIZE];
	char random[JSON_FIELD_RANDOM_MAXSIZE];
}trfr_respond_t;

typedef struct
{
	trans_action_t action;
	trfield_obj_t *obj;
	sn_t gw_sn;
	sn_t *dev_sns;
	int sn_size;
	char random[JSON_FIELD_RANDOM_MAXSIZE];
}trfr_refresh_t;

typedef struct
{
	trans_action_t action;
	trfield_obj_t *obj;
	sn_t gw_sn;
	trfield_ctrl_t **ctrls;
	int ctrl_size;
	char random[JSON_FIELD_RANDOM_MAXSIZE];
}trfr_control_t;

typedef struct
{
	trans_action_t action;
	trfield_obj_t *obj;
	trans_action_t req_action;
	char info[JSON_FIELD_INFO_MAXSIZE];
	char random[JSON_FIELD_RANDOM_MAXSIZE];
}trfr_tocolres_t;

char *get_action_to_str(trans_action_t action);

trfield_device_t *get_trfield_device_alloc(sn_t dev_sn, char *dev_data);

trfield_ctrl_t *get_trfield_ctrl_alloc(sn_t dev_sn, char *cmd);

trfield_obj_t *get_trfield_obj_alloc(char *owner, char *custom);

trfr_tocolreq_t *get_trfr_tocolreq_alloc(trfield_obj_t *obj, sn_t gw_sn, char *protocol, char *random);
void get_trfr_tocolreq_free(trfr_tocolreq_t *tocolreq);

trfr_report_t *get_trfr_report_alloc(trfield_obj_t *obj, sn_t gw_sn, trfield_device_t **devices, int dev_size, char *random);
void get_trfr_report_free(trfr_report_t *report);

trfr_check_t *get_trfr_check_alloc(trfield_obj_t *obj, sn_t gw_sn, sn_t dev_sns[], int sn_size, char *code_check, char *code_data, char *random);
void get_trfr_check_free(trfr_check_t *check);

trfr_respond_t *get_trfr_respond_alloc(trfield_obj_t *obj, sn_t gw_sn, sn_t dev_sn, char *dev_data, char *random);
void get_trfr_respond_free(trfr_respond_t *respond);

trfr_refresh_t *get_trfr_refresh_alloc(trfield_obj_t *obj, sn_t gw_sn, sn_t dev_sns[], int sn_size, char *random);
void get_trfr_refresh_free(trfr_refresh_t *refresh);

trfr_control_t *get_trfr_control_alloc(trfield_obj_t *obj, sn_t gw_sn, trfield_ctrl_t **ctrls, int ctrl_size, char *random);
void get_trfr_control_free(trfr_control_t *control);

trfr_tocolres_t *get_trfr_tocolres_alloc(trfield_obj_t *obj, trans_action_t req_action, char *info, char *random);
void get_trfr_tocolres_free(trfr_tocolres_t *tocolres);

#ifdef __cplusplus
}
#endif

#endif  //__FIELDLYSIS_H__

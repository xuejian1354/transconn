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

//app device type
#define FR_APP_CONNECTOR			"00"
#define FR_APP_ENDNODE				"00"
#define FR_APP_LIGHTSWITCH_ONE		"01"
#define FR_APP_LIGHTSWITCH_TWO		"02"
#define FR_APP_LIGHTSWITCH_THREE	"03"
#define FR_APP_LIGHTSWITCH_FOUR		"04"
#define FR_APP_HUELIGHT				"05"
#define FR_APP_ALARM				"11"
#define FR_APP_IR_DETECTION			"12"
#define FR_APP_DOOR_SENSOR			"13"
#define FR_APP_ENVDETECTION			"14"
#define FR_APP_IR_RELAY				"21"
#define FR_APP_AIRCONTROLLER		"F0"
#define FR_APP_RELAYSOCKET			"F1"
#define FR_APP_LIGHTDETECT			"F2"
#define FR_APP_HUMITURE_DETECTION	"A1"
#define FR_APP_SOLENOID_VALVE		"A2"

//type for smartlab device
#define FR_APP_LAMPSWITCH		"31"
#define FR_APP_PROJECTOR		"32"
#define FR_APP_AIRCONDITION		"33"
#define FR_APP_CURTAIN			"34"
#define FR_APP_DOORLOCK			"35"

#define JSON_FIELD_NAME_MAXSIZE			64
#define JSON_FIELD_OBJ_MAXSIZE			32
#define JSON_FIELD_SN_MAXSIZE			24
#define JSON_FIELD_DATA_MAXSIZE			32
#define JSON_FIELD_CMD_MAXSIZE			32
#define JSON_FIELD_TOCOL_MAXSIZE		8
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
	FRAPP_LIGHTSWITCH_ONE = 1,
	FRAPP_LIGHTSWITCH_TWO = 2,
	FRAPP_LIGHTSWITCH_THREE = 3,
	FRAPP_LIGHTSWITCH_FOUR = 4,
	FRAPP_HUELIGHT = 5,
	FRAPP_ALARM = 11,
	FRAPP_IR_DETECTION = 12,
	FRAPP_DOOR_SENSOR = 13,
	FRAPP_ENVDETECTION = 14,
	FRAPP_IR_RELAY = 21,
	FRAPP_AIRCONTROLLER = 0xF0,
	FRAPP_RELAYSOCKET = 0xF1,
	FRAPP_LIGHTDETECT = 0xF2,
	FRAPP_HUMITURE_DETECTION = 0xA1,
	FRAPP_SOLENOID_VALVE = 0xA2,

	//type for smartlab device
	FRAPP_LAMPSWITCH = 31,
	FRAPP_PROJECTOR = 32,
	FRAPP_AIRCONDITION = 33,
	FRAPP_CURTAIN = 34,
	FRAPP_DOORLOCK = 35,
	
	FRAPP_NONE = 0xFF,
}fr_app_type_t;

typedef enum
{
	TN_GW,
	TN_SW1,
	TN_SW2,
	TN_SW3,
	TN_SW4,
	TN_HUE,
	TN_ALARM,
	TN_IRDECT,
	TN_DOORS,
	TN_PM25,
	TN_IRRELAY,
	TN_AIRCON,
	TN_RELAY,
	TN_HT,
	TN_VALVE,
	TN_LAMPSW,
	TN_PJECTOR,
	TN_AIRCONDITION,
	TN_CURTAIN,
	TN_DOORLOCK,
	TN_NUMS
}type_name_t;

typedef struct
{
    fr_app_type_t type;
    char *val;
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

fr_app_type_t get_frapp_type_from_str(char *app_type);
char *get_frapp_type_to_str(fr_app_type_t app_type);

char *get_action_to_str(trans_action_t action);

trfield_device_t *get_trfield_device_alloc(char *name,
	sn_t dev_sn, char *dev_type, int znet_status, char *dev_data);

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

#endif  //__FIELDLYSIS_H__

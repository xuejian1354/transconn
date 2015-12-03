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
#include <protocol/framelysis.h>

#define JSON_FIELD_NAME_MAXSIZE		64
#define JSON_FIELD_SN_MAXSIZE		24
#define JSON_FIELD_DATA_MAXSIZE		32
#define JSON_FIELD_CMD_MAXSIZE		32
#define JSON_FIELD_TOCOL_MAXSIZE		8
#define JSON_FIELD_CODECHECK_MAXSIZE	8
#define JSON_FIELD_CODEDATA_MAXSIZE	40
#define JSON_FIELD_RANDOM_MAXSIZE	6

#define JSON_FIELD_ACTION		"action"
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
#define JSON_FIELD_RANDOM		"random"

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
	sn_t *dev_sns;
	int sn_size;
	char cmd[JSON_FIELD_CMD_MAXSIZE];
}trfield_ctrl_t;

typedef struct
{
	trans_action_t action;
	char protocol[JSON_FIELD_TOCOL_MAXSIZE];
	char random[JSON_FIELD_RANDOM_MAXSIZE];
}trfr_tocolreq_t;

typedef struct
{
	trans_action_t action;
	sn_t gw_sn;
	trfield_device_t **devices;
	int dev_size;
	char random[JSON_FIELD_RANDOM_MAXSIZE];
}trfr_report_t;

typedef struct
{
	trans_action_t action;
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
	sn_t gw_sn;
	sn_t dev_sn;
	char dev_data[JSON_FIELD_DATA_MAXSIZE];
	char random[JSON_FIELD_RANDOM_MAXSIZE];
}trfr_respond_t;

typedef struct
{
	trans_action_t action;
	sn_t gw_sn;
	sn_t *dev_sns;
	int sn_size;
	char random[JSON_FIELD_RANDOM_MAXSIZE];
}trfr_refresh_t;

typedef struct
{
	trans_action_t action;
	sn_t gw_sn;
	trfield_ctrl_t **ctrls;
	int ctrl_size;
	char random[JSON_FIELD_RANDOM_MAXSIZE];
}trfr_control_t;

typedef struct
{
	trans_action_t action;
	trans_action_t req_action;
	char random[JSON_FIELD_RANDOM_MAXSIZE];
}trfr_tocolres_t;

char *get_action_to_str(trans_action_t action);

trfield_device_t *get_trfield_device_alloc(char *name,
	sn_t dev_sn, char *dev_type, int znet_status, char *dev_data);

trfield_ctrl_t *get_trfield_ctrl_alloc(sn_t* dev_sns, int dev_size, char *cmd);

trfr_tocolreq_t *get_trfr_tocolreq_alloc(char *protocol, char *random);
void get_trfr_tocolreq_free(trfr_tocolreq_t *tocolreq);

trfr_report_t *get_trfr_report_alloc(sn_t gw_sn, trfield_device_t **devices, int dev_size, char *random);
void get_trfr_report_free(trfr_report_t *report);

trfr_check_t *get_trfr_check_alloc(sn_t gw_sn, sn_t dev_sns[], int sn_size, char *code_check, char *code_data, char *random);
void get_trfr_check_free(trfr_check_t *check);

trfr_respond_t *get_trfr_respond_alloc(sn_t gw_sn, sn_t dev_sn, char *dev_data, char *random);
void get_trfr_respond_free(trfr_respond_t *respond);

trfr_refresh_t *get_trfr_refresh_alloc(sn_t gw_sn, sn_t dev_sns[], int sn_size, char *random);
void get_trfr_refresh_free(trfr_refresh_t *refresh);

trfr_control_t *get_trfr_control_alloc(sn_t gw_sn, trfield_ctrl_t **ctrls, int ctrl_size, char *random);
void get_trfr_control_free(trfr_control_t *control);

trfr_tocolres_t *get_trfr_tocolres_alloc(trans_action_t req_action, char *random);
void get_trfr_tocolres_free(trfr_tocolres_t *tocolres);

#endif  //__FIELDLYSIS_H__

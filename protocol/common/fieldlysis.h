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

#define FIELD_NAME_MAXSIZE		12
#define FIELD_SN_MAXSIZE		24
#define FIELD_DATA_MAXSIZE		7
#define FIELD_CMD_MAXSIZE		7
#define FIELD_TOCOL_MAXSIZE		8
#define FIELD_CODECHECK_MAXSIZE	8
#define FIELD_CODEDATA_MAXSIZE	40

#define FIELD_ACTION	"action"
#define FIELD_PROTOCOL	"protocol"
#define FIELD_GWSN		"gw_sn"
#define FIELD_DEVICES	"devices"
#define FIELD_NAME		"name"
#define FIELD_DEVSN		"dev_sn"
#define FIELD_DEVSNS	"dev_sns"
#define FIELD_DEVTYPE	"dev_type"
#define FIELD_DEVDATA	"dev_data"
#define FIELD_ZSTATUS	"znet_status"
#define FIELD_DEVDATA	"dev_data"
#define FIELD_CODE		"code"
#define FIELD_CODECHECK	"code_check"
#define FIELD_CODEDATA	"code_data"
#define FIELD_CMD		"cmd"


#define FIELD_TOCOL_UDP		"udp"
#define FIELD_TOCOL_TCP		"tcp"
#define FIELD_TOCOL_HTTP	"http"

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

typedef char sn_t[FIELD_SN_MAXSIZE];

typedef struct
{
	char name[FIELD_NAME_MAXSIZE];
	sn_t dev_sn;
	fr_app_type_t dev_type;
	char znet_status;
	char dev_data[FIELD_DATA_MAXSIZE];
}trfield_device_t;

typedef struct
{
	trans_action_t action;
	char protocol[FIELD_TOCOL_MAXSIZE];
}trfr_tocolreq_t, trfr_tocolres_t;

typedef struct
{
	trans_action_t action;
	sn_t gw_sn;
	trfield_device_t **devices;
	int dev_size;
}trfr_report_t;

typedef struct
{
	trans_action_t action;
	sn_t gw_sn;
	sn_t *dev_sns;
	int sn_size;
	struct {
		char code_check[FIELD_CODECHECK_MAXSIZE];
		char code_data[FIELD_CODEDATA_MAXSIZE];
	} code;
}trfr_check_t;

typedef struct
{
	trans_action_t action;
	sn_t gw_sn;
	sn_t dev_sn;
	char dev_data[FIELD_DATA_MAXSIZE];
}trfr_respond_t;

typedef struct
{
	trans_action_t action;
	sn_t gw_sn;
	sn_t *dev_sns;
	int sn_size;
}trfr_refresh_t;

typedef struct
{
	trans_action_t action;
	sn_t gw_sn;
	sn_t *dev_sns;
	int sn_size;
	char cmd[FIELD_CMD_MAXSIZE];
}trfr_control_t;

trfield_device_t *get_trfield_device_alloc(char *name,
	sn_t dev_sn, char *dev_type, char *znet_status, char *dev_data);

trfr_tocolreq_t *get_trfr_tocolreq_alloc(trans_action_t action, char *protocol);
void get_trfr_tocolreq_free(trfr_tocolreq_t *report);

trfr_report_t *get_trfr_report_alloc(trans_action_t action,
	sn_t gw_sn, trfield_device_t **devices, int dev_size);
void get_trfr_report_free(trfr_report_t *report);

trfr_check_t *get_trfr_check_alloc(trans_action_t action,
	sn_t gw_sn, sn_t dev_sns[], int sn_size, char *code_check, char *code_data);
void get_trfr_check_free(trfr_check_t *check);

trfr_respond_t *get_trfr_respond_alloc(trans_action_t action,
	sn_t gw_sn, sn_t dev_sn, char *dev_data);
void get_trfr_respond_free(trfr_respond_t *respond);

trfr_refresh_t *get_trfr_refresh_alloc(trans_action_t action,
	sn_t gw_sn, sn_t dev_sns[], int sn_size);
void get_trfr_refresh_free(trfr_refresh_t *refresh);

trfr_control_t *get_trfr_control_alloc(trans_action_t action,
	sn_t gw_sn, sn_t dev_sns[], int sn_size, char *cmd);
void get_trfr_control_free(trfr_control_t *control);

#endif  //__FIELDLYSIS_H__

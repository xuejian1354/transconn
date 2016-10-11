/*
 * request.h
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

#ifndef __REQUEST_H__
#define __REQUEST_H__

#include <services/globals.h>
#include <module/netapi.h>
#include <protocol/devices.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*respond_request_t)(trfr_respond_t *);

typedef struct RespondData
{
	sn_t sn;
	char dev_data[JSON_FIELD_DATA_MAXSIZE];
	char random[JSON_FIELD_RANDOM_MAXSIZE];
	int  timer_id;
	respond_request_t respond_callback;

	struct RespondData *next;
}respond_data_t;

char *gen_current_checkcode();

void trans_send_tocolreq_request(trfr_tocolreq_t *tocolreq);
void trans_send_report_request(trfr_report_t *report);
void trans_send_check_request(trfr_check_t *check);
void trans_send_respond_request(trfr_respond_t *respond);

void trans_refresh_handler(trfr_refresh_t *refresh);
void trans_control_handler(trfr_control_t *control);

void trans_tocolres_handler(trfr_tocolres_t *tocolres);

void trans_send_tocolres_request(trfr_tocolres_t *tocolres);

void upload_data(char *random);
void device_ctrl(sn_t sn, char *cmd, char *random, respond_request_t callback);

void trans_send_frame_request(char *frame);

#ifdef __cplusplus
}
#endif

#endif  //__REQUEST_H__

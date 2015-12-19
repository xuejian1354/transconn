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
#include <protocol/old/devalloc.h>
#include <module/netapi.h>
#include <protocol/common/fieldlysis.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*respond_request_t)(frhandler_arg_t *, trfr_respond_t *);

typedef struct RespondData
{
	sn_t sn;
	char dev_data[JSON_FIELD_DATA_MAXSIZE];
	char random[JSON_FIELD_RANDOM_MAXSIZE];
	int  timer_id;
	respond_request_t respond_callback;

	struct RespondData *next;
}respond_data_t;

#ifdef COMM_CLIENT
void trans_send_tocolreq_request(frhandler_arg_t *arg, trfr_tocolreq_t *tocolreq);
void trans_send_report_request(frhandler_arg_t *arg, trfr_report_t *report);
void trans_send_check_request(frhandler_arg_t *arg, trfr_check_t *check);
void trans_send_respond_request(frhandler_arg_t *arg, trfr_respond_t *respond);
#endif
#if defined(COMM_CLIENT) || defined(DE_TRANS_UDP_CONTROL)
void trans_refresh_handler(frhandler_arg_t *arg, trfr_refresh_t *refresh);
void trans_control_handler(frhandler_arg_t *arg, trfr_control_t *control);
#endif

void trans_tocolres_handler(frhandler_arg_t *arg, trfr_tocolres_t *tocolres);

#ifdef COMM_SERVER
void trans_tocolreq_handler(frhandler_arg_t *arg, trfr_tocolreq_t *tocolreq);
void trans_report_handler(frhandler_arg_t *arg, trfr_report_t *report);
void trans_check_handler(frhandler_arg_t *arg, trfr_check_t *check);
void trans_respond_handler(frhandler_arg_t *arg, trfr_respond_t *respond);

void trans_send_refresh_request(frhandler_arg_t *arg, trfr_refresh_t *refresh);
void trans_send_control_request(frhandler_arg_t *arg, trfr_control_t *control);
#endif

void trans_send_tocolres_request(frhandler_arg_t *arg, trfr_tocolres_t *tocolres);

#ifdef COMM_CLIENT
void sync_gateway_info(gw_info_t *pgw_info);
void sync_zdev_info(uint8 isrefresh, dev_info_t *pdev_info);
void upload_data(uint8 isrefresh, char *random);
void device_ctrl(sn_t sn, char *cmd, char *random, respond_request_t callback);
#endif

#ifdef DE_TRANS_UDP_CONTROL
void detrans_send_refresh(sn_t devsn);
void detrans_send_control(sn_t devsn, char *cmd);
#endif

void trans_send_frame_request(frhandler_arg_t *arg, trans_action_t action, char *frame);

#ifdef __cplusplus
}
#endif

#endif  //__REQUEST_H__

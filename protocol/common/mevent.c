/*
 * mevent.c
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

#include "mevent.h"
#include <protocol/protocol.h>
#include <protocol/request.h>
#include <module/serial.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TIMER_SUPPORT

static void upcheck(void *p);
static void query_dev(void *p);

void gateway_init()
{
	//Upcheck with server
	set_upcheck_reset();

	//Query Devices
	timer_event_param_t timer_param;
	timer_param.resident = 1;
	timer_param.interval = get_global_conf()->querydev_time;
	timer_param.count = 1;
	timer_param.immediate = 1;

	set_mevent(GATEWAY_QUERYDEV_EVENT, query_dev, &timer_param);
}

void set_upcheck_reset()
{
	timer_event_param_t timer_param;
	timer_param.resident = 1;
	timer_param.interval = get_global_conf()->upcheck_time;
	timer_param.count = 1;
	timer_param.immediate = 0;

	set_mevent(GATEWAY_UPCHECK_EVENT, upcheck, &timer_param);
}

static void upcheck(void *p)
{
	sn_t gwno_str = {0};
	incode_xtocs(gwno_str,
					get_gateway_info()->gw_no,
					sizeof(zidentify_no_t));

	char *cur_code = gen_current_checkcode();

	trfr_check_t *check = get_trfr_check_alloc(NULL,
												gwno_str,
												NULL,
												0,
												(char *)"md5",
												cur_code,
												NULL);

	trans_send_check_request(check);
	get_trfr_check_free(check);
}

void query_dev(void *p)
{
    static unsigned int i = 0;

	trbuffer_t *frame = get_devopt_frame_alloc(i++, 0, 2);
	serial_write(frame->data, frame->len);
	get_devopt_frame_free(frame);

	if(i > get_global_conf()->querydev_num)
	{
		i = 1;
	}
}

void set_mevent(int id, timer_callback_t event_callback, timer_event_param_t *param)
{
	timer_event_t *timer_event = (timer_event_t *)calloc(1, sizeof(timer_event_t));
	timer_event->timer_id = id;
	timer_event->param = *param;
	timer_event->timer_callback = event_callback;

	if(set_timer_event(timer_event) != 0)
	{
		free(timer_event);
	}
}
#endif

#ifdef __cplusplus
}
#endif

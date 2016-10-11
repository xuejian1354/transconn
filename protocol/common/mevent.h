/*
 * mevent.h
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
#ifndef __MEVENT_H__
#define __MEVENT_H__

#include <services/globals.h>
#include <services/etimer.h>
#include <protocol/protocol.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GATEWAY_UPCHECK_EVENT	0x0001
#define GATEWAY_QUERYDEV_EVENT	0x0002
#define ZDEVICE_RESPOND_EVENT	0x0003

#ifdef TIMER_SUPPORT
void gateway_init();
void set_upcheck_reset();

void set_mevent(int id, timer_callback_t event_callback, timer_event_param_t *param);
#endif

#ifdef __cplusplus
}
#endif
 
 #endif  //__MEVENT_H__

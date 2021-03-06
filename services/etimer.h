/*
 * etimer.h
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
#ifndef __ETIMER_H__
#define __ETIMER_H__

#include <services/globals.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TIMER_SUPPORT

typedef void (*timer_callback_t)(void *);

typedef struct
{
	unsigned long interval;
	int count;
	int resident;
	int immediate;
	void *arg;
}timer_event_param_t;

typedef struct Timer_Event
{
	int timer_id;
	unsigned long interval_count;
	timer_event_param_t param;
	timer_callback_t timer_callback;
	struct Timer_Event *next;
}timer_event_t;

int timer_init();
int set_timer_event(timer_event_t *event);
timer_event_t *query_timer_event(int timer_id);
int del_timer_event(int timer_id);
#endif

#ifdef __cplusplus
}
#endif

#endif  //__ETIMER_H__
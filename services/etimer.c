/*
 * etimer.c
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

#include "etimer.h"
#include <signal.h>
#include <sys/time.h>
#include <tpool.h>

#define TIMER_INTERVAL	3

static timer_event_t *p_event;

void timer_func(int sig)
{
	timer_event_t *t_event = p_event;
	
	while(t_event != NULL)
	{
		tpool_add_work(t_event->timer_callback, t_event->arg);
		if(!t_event->resident && !(--(t_event->count)))
		{
			del_timer_event(t_event->timer_id);
		}
		
		t_event = t_event->next;
	}
}


int timer_initial()
{
	p_event = NULL;
	
	struct itimerval tv, oldtv;
	signal(SIGALRM, timer_func);

    tv.it_interval.tv_sec = TIMER_INTERVAL;  
    tv.it_interval.tv_usec = 0;  
    tv.it_value.tv_sec = TIMER_INTERVAL;  
    tv.it_value.tv_usec = 0;  
    if(setitimer(ITIMER_REAL, &tv, &oldtv) < 0)
    {
		perror("timer initial");
		return -1;
	}
}

int set_timer_event(timer_event_t *event)
{
	timer_event_t *pre_event = NULL;
	timer_event_t *t_event = p_event;

	if(event == NULL)
	{
		return -1;
	}
	else
	{
		event->next = NULL;
	}
	
	if(t_event == NULL)
	{
		t_event = event;
		return 0;
	}

	while(t_event != NULL)
	{
		if(t_event->timer_id == event->timer_id)
		{
			t_event->count = event->count;
			t_event->resident = event->resident;
			t_event->arg = event->arg;
			t_event->timer_callback = event->timer_callback;
			
			return 1;
		}
		pre_event = t_event;
		t_event = t_event->next;
	}

	pre_event->next = event;
	return 0;
}

int del_timer_event(int timer_id)
{
	timer_event_t *pre_event = NULL;
	timer_event_t *t_event = p_event;
	
	if(t_event == NULL)
	{
		return -1;
	}

	while(t_event != NULL)
	{
		if(t_event->timer_id == timer_id)
		{
			if(pre_event != NULL)
			{
				pre_event->next = t_event->next;
				
			}
			else
			{
				p_event = p_event->next;
			}

			free(t_event);
			return 0;
		}
		pre_event = t_event;
		t_event = t_event->next;
	}

	return -1;
}

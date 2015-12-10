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
#include <errno.h>
#include <signal.h>
#include <sys/time.h>

#ifdef TIMER_SUPPORT

#define TIMER_MIN_INTERVAL	1000

static timer_event_t *p_event;
static pthread_mutex_t timer_lock;

void *timer_func(void *p)
{
	while(1)
	{
		timer_event_t *t_event = p_event;
		
		while(t_event != NULL)
		{
			if(t_event->param.immediate)
			{
				t_event->interval_count = 0;
				t_event->param.immediate = 0;
			}
			else
			{
				t_event->interval_count--;
			}
			
			if(t_event->interval_count <= 0)
			{
				t_event->interval_count = t_event->param.interval;
				t_event->param.count--;
				tpool_add_work(t_event->timer_callback, t_event->param.arg, TPOOL_NONE);
			}
			
			if(!t_event->param.resident && !t_event->param.count)
			{
				del_timer_event(t_event->timer_id);
			}
			
			t_event = t_event->next;
		}

		usleep(TIMER_MIN_INTERVAL);
	}
}


int timer_init()
{
	p_event = NULL;

	if(pthread_mutex_init(&timer_lock, NULL) != 0)
    {
        DE_PRINTF(1, "%s: pthread_mutext_init failed, errno:%d, error:%s\n",
            __FUNCTION__, errno, strerror(errno));
        return -1;
    }

	/* Unused cause cutting by curl_easy_perform() */
	/*struct itimerval tv, oldtv;
	signal(SIGALRM, timer_func);

	tv.it_interval.tv_sec = 0;  
	tv.it_interval.tv_usec = TIMER_MIN_INTERVAL;  
	tv.it_value.tv_sec = 0;  
	tv.it_value.tv_usec = TIMER_MIN_INTERVAL;  
	if(setitimer(ITIMER_REAL, &tv, &oldtv) < 0)
	{
		perror("timer initial");
		return -1;
	}*/

	pthread_t pthtimer;
	pthread_create(&pthtimer, NULL, timer_func, NULL);
	
	return 0;
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
		event->interval_count = event->param.interval;
		event->next = NULL;
	}
	
	if(t_event == NULL)
	{
		p_event = event;
		return 0;
	}

	while(t_event != NULL)
	{
		if(t_event->timer_id == event->timer_id)
		{
			pthread_mutex_lock(&timer_lock);
			t_event->param = event->param;
			t_event->interval_count = t_event->param.interval;
			t_event->timer_callback = event->timer_callback;
			pthread_mutex_unlock(&timer_lock);
			return 1;
		}
		pre_event = t_event;
		t_event = t_event->next;
	}

	pre_event->next = event;
	return 0;
}

timer_event_t *query_timer_event(int timer_id)
{
	timer_event_t *t_event = p_event;

	while(t_event != NULL)
	{
		if(t_event->timer_id != timer_id)
		{
			t_event = t_event->next;
		}
		else
		{
			return t_event;
		}
	}

	return NULL;
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
			pthread_mutex_lock(&timer_lock);
			if(pre_event != NULL)
			{
				pre_event->next = t_event->next;
				
			}
			else
			{
				p_event = p_event->next;
			}
			free(t_event);
			pthread_mutex_unlock(&timer_lock);

			return 0;
		}
		pre_event = t_event;
		t_event = t_event->next;
	}

	return -1;
}

#endif

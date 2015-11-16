/*
 * tpool.h
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
#ifndef __T_POOL_H__
#define __T_POOL_H__

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

typedef enum
{
	TPOOL_NONE,
	TPOOL_LOCK,
}tpool_opt_t;

typedef void (*routine_t)(void *);

/* excute task list */
typedef struct tpool_work {
    routine_t 	routine;	/* task func */
    void		*arg;		/* task param */
    tpool_opt_t options;
    struct tpool_work	*next;
} tpool_work_t;

typedef struct tpool {
    int			shutdown;	/* is destory */
    int			max_thr_num;	/* max thread num */
    pthread_t		*thr_id;	/* thread id array */
    tpool_work_t	*queue_head;	/* thread task list */
    pthread_mutex_t	queue_lock;
    pthread_mutex_t	func_lock;
    pthread_cond_t	queue_ready;
} tpool_t;

int tpool_create(int max_thr_num);
void tpool_destroy();
int tpool_add_work(routine_t routine, void *arg, tpool_opt_t options);
#endif //__TPOOL_H__

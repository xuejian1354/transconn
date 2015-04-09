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

/* excute task list */
typedef struct tpool_work {
    void *(*routine)(void *);	/* task func */
    void		*arg;		/* task param */
    struct tpool_work	*next;
} tpool_work_t;

typedef struct tpool {
    int			shutdown;	/* is destory */
    int			max_thr_num;	/* max thread num */
    pthread_t		*thr_id;	/* thread id array */
    tpool_work_t	*queue_head;	/* thread task list */
    pthread_mutex_t	queue_lock;
    pthread_cond_t	queue_ready;
} tpool_t;


int tpool_create(int max_thr_num);
void tpool_destroy();
int tpool_add_work(void *(*routine)(void *), void *arg);
#endif //__TPOOL_H__

/*
 * tpool.c
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
#include "tpool.h"

static tpool_t *tpool = NULL;

static void *thread_routine(void *arg)
{
    tpool_work_t *work;

    while(1)
    {
        pthread_mutex_lock(&tpool->queue_lock);
        while(!tpool->queue_head && !tpool->shutdown)
        {
            pthread_cond_wait(&tpool->queue_ready, &tpool->queue_lock);
        }

        if(tpool->shutdown)
        {
            pthread_mutex_unlock(&tpool->queue_lock);
            pthread_exit(NULL);
        }

        work = tpool->queue_head;
        tpool->queue_head = tpool->queue_head->next;
        pthread_mutex_unlock(&tpool->queue_lock);

        work->routine(work->arg);
        free(work);
    }

    return NULL;
}

int tpool_create(int max_thr_num)
{
    int i;
    
    tpool = calloc(1, sizeof(tpool_t));
    if(!tpool)
    {
        fprintf(stderr, "%s: calloc failed\n", __FUNCTION__);
        return -1;
    }

    tpool->max_thr_num = max_thr_num;
    tpool->shutdown = 0;
    tpool->queue_head = NULL;
    if(pthread_mutex_init(&tpool->queue_lock, NULL) != 0)
    {
        fprintf(stderr, "%s: pthread_mutext_init failed, errno:%d, error:%s\n",
            __FUNCTION__, errno, strerror(errno));
        return -1;
    }

    tpool->thr_id = calloc(max_thr_num, sizeof(pthread_t));
    if(!tpool->thr_id)
    {
        fprintf(stderr, "%s: calloc failed\n", __FUNCTION__);
        return -1;
    }

    for(i=0; i<max_thr_num; i++)
    {
        if(pthread_create(&tpool->thr_id[i], NULL, thread_routine, NULL) != 0)
        {
            fprintf(stderr, "%s: pthread_create failed, errno:%d, erro:%s\n", 
                __FUNCTION__, errno, strerror(errno));
            return -1;
        }
    }

    return 0;
}

void tpool_destroy()
{
    int i;
    tpool_work_t *member;

    if(tpool->shutdown)
    {
        return;
    }
    tpool->shutdown = 1;

    pthread_mutex_lock(&tpool->queue_lock);
    pthread_cond_broadcast(&tpool->queue_ready);
    pthread_mutex_unlock(&tpool->queue_lock);

    for(i=0; i<tpool->max_thr_num; i++)
    {
        pthread_join(tpool->thr_id[i], NULL);
    }
    free(tpool->thr_id);

    while(tpool->queue_head)
    {
        member = tpool->queue_head;
        tpool->queue_head = tpool->queue_head->next;
        free(member);
    }

    pthread_mutex_destroy(&tpool->queue_lock);
    pthread_cond_destroy(&tpool->queue_ready);

    free(tpool);
}


int tpool_add_work(void *(*routine)(void *), void *arg)
{
    tpool_work_t *work, *member;

    if(!routine)
    {
        fprintf(stderr, "%s: Invalid argument\n", __FUNCTION__);
        return -1;
    }

    work = malloc(sizeof(tpool_work_t));
    if(!work)
    {
        fprintf(stderr, "%s: malloc failed\n", __FUNCTION__);
        return -1;
    }
    work->routine = routine;
    work->arg = arg;
    work->next = NULL;

    pthread_mutex_lock(&tpool->queue_lock);
    member = tpool->queue_head;
    if(!member)
    {
        tpool->queue_head = work;
    }
    else
    {
        while(member->next)
        {
            member = member->next;
        }
        member->next = work;
    }
    pthread_cond_signal(&tpool->queue_ready);
    pthread_mutex_unlock(&tpool->queue_lock);

    return 0;
}

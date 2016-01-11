/*
 * dballoc.h
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
#ifndef __DBALLOC_H__
#define __DBALLOC_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings_t.h>

#ifdef __cplusplus
extern "C" {
#endif

#define areas_t strings_t

typedef struct
{
	char **sn;
	int *iscollects;
	char **locates;
	unsigned int size;
}devices_t;

typedef struct
{
	char *name;
	char **devices;
	unsigned int dev_size;
	int *func_ids;
	unsigned int func_size;
	char **params;
	unsigned int param_size;
}scene_t;

typedef struct
{
	scene_t **scenes;
	unsigned int size;
}scenes_t;
	
devices_t *devices_alloc(unsigned int size);
int devices_add(devices_t *devs, char *sn, int iscollect, char *locate);
void devices_free(devices_t *devs);

areas_t *areas_alloc(unsigned int size);
int areas_add(areas_t *areas, char * area);
void areas_free(areas_t *areas);

scene_t *scene_create(char *name, char **devices, unsigned int dev_size, 
	int *func_ids, unsigned int func_size, char **params, unsigned int param_size);
void scene_free(scene_t *scene);

scenes_t *scenes_alloc(unsigned int size);
int scenes_put(scenes_t *scenes, scene_t *scene);
void scenes_free(scenes_t *scenes);

#ifdef __cplusplus
}
#endif

#endif	//__DBALLOC_H__
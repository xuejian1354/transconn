/*
 * dballoc.c
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
#include "dballoc.h"

#ifdef __cplusplus
extern "C" {
#endif

devices_t *devices_alloc(unsigned int size)
{
	if(size == 0)
	{
		return NULL;
	}

	devices_t *devices = (devices_t *)calloc(1, sizeof(devices_t));
	devices->size = size;
	devices->sn = (char **)calloc(size, sizeof(char *));
	devices->iscollects = (int *)calloc(size, sizeof(int));
	devices->locates = (char **)calloc(size, sizeof(char *));

	return devices;
}

int devices_add(devices_t *devs, char *sn, int iscollect, char *locate)
{
	int i;
	if(devs == NULL || devs->sn == NULL || devs->size == 0)
	{
		return -1;
	}

	for(i=0; i<devs->size; i++)
	{
		if(*(devs->sn+i) == NULL)
		{
			break;
		}
	}

	if(i == devs->size)
	{
		return -1;
	}

	int tlen = strlen(sn);
	*(devs->sn+i)= (char *)calloc(tlen+1, sizeof(char));
	strncpy(*(devs->sn+i), sn, tlen);

	*(devs->iscollects+i) = iscollect;

	if(locate != NULL)
	{
		int llen = strlen(locate);
		*(devs->locates+i) = (char *)calloc(llen+1, sizeof(char));
		strncpy(*(devs->locates+i), locate, llen);
	}

	return 0;
}

void devices_free(devices_t *devs)
{
	if(devs == NULL)
	{
		return;
	}

	if(devs->sn != NULL)
	{
		int i = 0;
		while(i < devs->size)
		{
			free(*(devs->sn+i));
			i++;
		}
	}

	if(devs->locates != NULL)
	{
		int i = 0;
		while(i < devs->size)
		{
			free(*(devs->locates+i));
			i++;
		}
	}

	free(devs->sn);
	free(devs->iscollects);
	free(devs->locates);
	free(devs);
}

areas_t *areas_alloc(unsigned int size)
{
	return strings_alloc(size);
}

int areas_add(areas_t *areas, char * area)
{
	return strings_add(areas, area);
}

void areas_free(areas_t *areas)
{
	return strings_free(areas);
}

scene_t *scene_create(char *name, char **devices, unsigned int dev_size, 
	int *func_ids, unsigned int func_size, char **params, unsigned int param_size)
{
	if(name == NULL)
	{
		return NULL;
	}

	scene_t *scene = (scene_t *)calloc(1, sizeof(scene_t));

	int nlen = strlen(name);
	scene->name = (char *)calloc(nlen+1, sizeof(char));
	strncpy(scene->name, name, nlen);

	if(devices != NULL && dev_size != 0)
	{
		int i;
		scene->devices = (char **)calloc(dev_size, sizeof(char *));
		for(i=0; i<dev_size; i++)
		{
			char *dev_src = *(devices+i);
			int dlen = strlen(dev_src);
			if(dev_src != NULL && dlen > 0)
			{
				*(scene->devices+i) = (char *)calloc(dlen+1, sizeof(char));
				memcpy(*(scene->devices+i), dev_src, dlen);
			}
		}
	}

	if(func_ids != NULL && func_size != 0)
	{
		scene->func_ids = (int *)calloc(func_size, sizeof(int));
		memcpy(scene->func_ids, func_ids, func_size);
	}

	if(params != NULL && param_size != 0)
	{
		int i;
		scene->params = (char **)calloc(param_size, sizeof(char *));
		for(i=0; i<param_size; i++)
		{
			char *param_src = *(params+i);
			int plen = strlen(param_src);
			if(param_src != NULL && plen > 0)
			{
				*(scene->params+i) = (char *)calloc(plen+1, sizeof(char));
				memcpy(*(scene->params+i), param_src, plen);
			}
		}
	}
}

scenes_t *scenes_alloc(unsigned int size)
{
	if(size == 0)
	{
		return NULL;
	}

	scenes_t *scenes = (scenes_t *)calloc(1, sizeof(scenes_t));
	scenes->size = size;
	scenes->scenes = (scene_t **)calloc(size, sizeof(scene_t *));

	return scenes;
}

int scenes_put(scenes_t *scenes, scene_t *scene)
{
	int i;
	if(scenes == NULL || scenes->scenes == NULL || scenes->size == 0)
	{
		return -1;
	}

	for(i=0; i<scenes->size; i++)
	{
		if(*(scenes->scenes+i) == NULL)
		{
			break;
		}
	}

	if(i == scenes->size)
	{
		return -1;
	}

	if(scene == NULL || scene->name == NULL)
	{
		return -1;
	}

	*(scenes->scenes+i) = scene;
	return 0;
}

void scene_free(scene_t *scene)
{
	if(scene == NULL)
	{
		return;
	}

	free(scene->name);

	char **devices = scene->devices;
	int dev_size = scene->dev_size;
	if(devices != NULL && dev_size > 0)
	{
		int i = 0;
		for(i=0; i<dev_size; i++)
		{
			free(*(devices+i));
		}
	}
	free(devices);

	free(scene->func_ids);

	char **params = scene->params;
	int param_size = scene->param_size;
	if(params != NULL && param_size > 0)
	{
		int i = 0;
		for(i=0; i<param_size; i++)
		{
			free(*(params+i));
		}
	}
	free(params);
	
	free(scene);
}

void scenes_free(scenes_t *scenes)
{
	int i;
	if(scenes == NULL || scenes->size == 0)
	{
		return;
	}

	for(i=0; i<scenes->size; i++)
	{
		scene_free(*(scenes->scenes+i));
	}
}

#ifdef __cplusplus
}
#endif

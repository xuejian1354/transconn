/*
 * strings_t.c
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
#include "strings_t.h"

strings_t *strings_alloc(unsigned int size)
{
	if(size == 0)
	{
		return NULL;
	}

	strings_t *strs = calloc(1, sizeof(strings_t));
	strs->size = size;
	strs->str = calloc(size, sizeof(char *));
	return strs;
}

int strings_add(strings_t *strs, char *str)
{
	int i;
	if(strs == NULL || strs->str == NULL || strs->size == 0)
	{
		return -1;
	}

	for(i=0; i<strs->size; i++)
	{
		if(*(strs->str+i) == NULL)
		{
			break;
		}
	}

	if(i == strs->size)
	{
		return -1;
	}

	int tlen = strlen(str);
	*(strs->str+i )= calloc(tlen+1, sizeof(char));;
	strncpy(*(strs->str+i), str, tlen);

	return 0;
}

void strings_free(strings_t *strs)
{
	int i = 0;

	if(strs == NULL)
	{
		return;
	}

	if(strs->str == NULL)
	{
		free(strs);
		return;
	}

	while(i < strs->size)
	{
		free(*(strs->str+i));
		i++;
	}

	free(strs->str);
	free(strs);
}
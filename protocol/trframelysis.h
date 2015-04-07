/*
 * trframelysis.h
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

#ifndef __TRFRAMELYSIS_H__
#define __TRFRAMELYSIS_H__

#include <mconfig.h>

typedef enum
{
	TRHEAD_PI,
	TRHEAD_BI,
	TRHEAD_GP,
	TRHEAD_RP,
	TRHEAD_GD,
	TRHEAD_RD,
	TRHEAD_DC,
	TRHEAD_UB,
	TRHEAD_NONE,
}tr_head_type_t;

typedef struct
{
	uint8 head[3];   //PI:
	zidentify_no_t zidentify_no;
	uint8 tail[4];    //:O/r/n
}pi_t;


typedef struct
{
	uint8 head[3];   //BI:
	uint8 trans_type;
	uint8 tail[4];
}bi_t;


typedef struct
{
	uint8 head[3];   //gp:
	zidentify_no_t zidentify_no;
	cidentify_no_t cidentify_no;
	uint8 tail[4];    //:O/r/n
}gp_t;


typedef struct
{
	uint8 head[3];   //rp:
	zidentify_no_t zidentify_no;
	cidentify_no_t cidentify_no;
	uint8 trans_type;
	uint8 ipaddr[16];
	uint8 port[2];
	uint8 tail[4];    //:O/r/n
}rp_t;


typedef struct
{
	uint8 head[3];   //gd:, rd:
	uint8 tail[4];    //:O/r/n
}gd_t,rd_t;

typedef struct
{
	uint8 head[3];   //dc:, ub:
	uint8 *data;
	uint8 datalen;
	uint8 tail[4];    //:O/r/n
}dc_t, ub_t;

void *get_trframe_alloc(tr_head_type_t head_type, uint8 buffer[], int length);
 
#endif  //__TRFRAMELYSIS_H__
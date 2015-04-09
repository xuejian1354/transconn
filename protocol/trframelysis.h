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

#include <services/globals.h>

//traversal frame head
#define TR_HEAD_PI	"PI:"	//gw put info to server
#define TR_HEAD_BI	"BI:"	//server back info to gw
#define TR_HEAD_GP	"GP:"	//capps get ip port from server
#define TR_HEAD_RP	"RP:"	//server return ip port to capps & gw
#define TR_HEAD_GD	"GD:"	//get traversal connection
#define TR_HEAD_RD	"RD:"	//return traversal connection
#define TR_HEAD_DC	"DC:"	//down control
#define TR_HEAD_UB	"UB:"	//up back

#define TR_TYPE_UDP	'1'
#define TR_TYPE_TCP	'2'

#define TR_FRAME_CON	'1'
#define TR_FRAME_REG	'2'
#define TR_FRAME_GET	'3'
#define TR_FRAME_PUT	'4'

#define TR_PI_DATA_FIX_LEN	25
#define TR_BI_DATA_FIX_LEN	25
#define TR_GP_DATA_FIX_LEN	31
#define TR_RP_DATA_FIX_LEN	36
#define TR_GD_DATA_FIX_LEN	7
#define TR_RD_DATA_FIX_LEN	7
#define TR_DC_DATA_FIX_LEN	7
#define TR_UB_DATA_FIX_LEN	7

#define TR_BUFFER_SIZE 	1024
#define TR_TAIL ":O\r\n"

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

typedef enum
{
	TRTYPE_UDP,
	TRTYPE_TCP,
	TRTYPE_NONE,
}tr_trans_type_t;

typedef enum
{
	TRFRAME_CON,
	TRFRAME_REG,
	TRFRAME_GET,
	TRFRAME_PUT,
	TRFRAME_NONE,
}tr_frame_type_t;

typedef struct
{
	uint8 sn[8];
	tr_trans_type_t trans_type;
	tr_frame_type_t fr_type;
	uint16 data_len;
	uint8 *data;
}pi_t;


typedef struct
{
	uint8 sn[8];
	tr_trans_type_t trans_type;
	tr_frame_type_t fr_type;
	uint16 data_len;
	uint8 *data;
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
	uint8 port[4];
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

typedef struct
{
	uint8 *data;
	uint8 size;
}tr_buffer_t;

void *get_trframe_alloc(tr_head_type_t head_type, uint8 buffer[], int length);
tr_buffer_t *get_trbuffer_alloc(tr_head_type_t type, void *frame);
void get_trbuffer_free(tr_buffer_t *p);
 
#endif  //__TRFRAMELYSIS_H__
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

#define TR_TYPE_UDP_NORMAL		'1'
#define TR_TYPE_UDP_TRAVERSAL	'2'
#define TR_TYPE_TCP_LONG		'3'
#define TR_TYPE_TCP_SHORT		'4'

#define TR_FRAME_CON		'1'
#define TR_FRAME_REG		'2'
#define TR_FRAME_GET		'3'
#define TR_FRAME_PUT_GW		'4'
#define TR_FRAME_PUT_DEV	'5'
#define TR_FRAME_TRANS		'6'

#define TR_INFO_NONE		'0'
#define TR_INFO_IP			'1'
#define TR_INFO_DATA		'2'
#define TR_INFO_UPDATE		'3'
#define	TR_INFO_CUT			'4'
#define TR_INFO_REG			'5'
#define TR_INFO_HOLD		'6'
#define TR_INFO_FOUND		'7'
#define TR_INFO_DISMATCH	'8'
#define TR_INFO_CONTROL		'9'
#define TR_INFO_QUERY		'A'
#define TR_INFO_REDATA		'B'

#define TR_PI_DATA_FIX_LEN	25
#define TR_BI_DATA_FIX_LEN	25
#define TR_GP_DATA_FIX_LEN	41
#define TR_RP_DATA_FIX_LEN	41
#define TR_GD_DATA_FIX_LEN	41
#define TR_RD_DATA_FIX_LEN	41
#define TR_DC_DATA_FIX_LEN	41
#define TR_UB_DATA_FIX_LEN	41

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
	TRTYPE_UDP_NORMAL,
	TRTYPE_UDP_TRAVERSAL,
	TRTYPE_TCP_LONG,
	TRTYPE_TCP_SHORT,
	TRTYPE_NONE,
}tr_trans_type_t;

typedef enum
{
	TRFRAME_CON,
	TRFRAME_REG,
	TRFRAME_GET,
	TRFRAME_PUT_GW,
	TRFRAME_PUT_DEV,
	TRFRAME_TRANS,
	TRFRAME_NONE,
}tr_frame_type_t;

typedef enum
{
	TRINFO_IP,
	TRINFO_DATA,
	TRINFO_UPDATE,
	TRINFO_CUT,
	TRINFO_REG,
	TRINFO_HOLD,
	TRINFO_FOUND,
	TRINFO_DISMATCH,
	TRINFO_CONTROL,
	TRINFO_QUERY,
	TRINFO_REDATA,
	TRINFO_NONE,
}tr_info_type_t;

typedef struct
{
	uint8 sn[8];
	tr_trans_type_t trans_type;
	tr_frame_type_t fr_type;
	int data_len;
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
	zidentify_no_t zidentify_no;
	cidentify_no_t cidentify_no;
	tr_trans_type_t trans_type;
	tr_info_type_t tr_info;
	uint16 data_len;
	uint8 *data;
}gp_t, rp_t, gd_t, rd_t, dc_t, ub_t;


typedef struct
{
	uint8 *data;
	uint8 size;
}tr_buffer_t;

tr_head_type_t get_trhead_from_str(char *head);

tr_trans_type_t get_trtrans_from_ch(char trans);
char get_trtrans_to_ch(tr_trans_type_t trans);

tr_frame_type_t get_trframe_from_ch(char trfra);
char get_trframe_to_ch(tr_frame_type_t trfra);

tr_info_type_t get_trinfo_from_ch(char trinfo);
char get_trinfo_to_ch(tr_info_type_t trinfo);


void *get_trframe_alloc(tr_head_type_t head_type, uint8 buffer[], int length);
void get_trframe_free(tr_head_type_t head_type, void *p);

tr_buffer_t *get_trbuffer_alloc(tr_head_type_t type, void *frame);
void get_trbuffer_free(tr_buffer_t *p);
 
#endif  //__TRFRAMELYSIS_H__
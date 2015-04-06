/*
 * protocol.c
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
#include "protocol.h"
#include <mincode.h>

static gw_info_t gw_info = 
{
	{0},
	0,
	0,
	NULL,
};

void analysis_ssa_frame(char *buf, int len)
{
	uc_t *uc;
	uo_t *uo;
	uh_t *uh;
	ur_t *ur;
	de_t *de;
	
	fr_head_type_t head_type = get_frhead_from_str(buf);
	
	void *p = get_frame_alloc(head_type, buf, len);

	if(p == NULL)
	{
		return;
	}

	switch(head_type)
	{
	case HEAD_UC:
		uc = (uc_t *)p;
		incode_ctoxs(gw_info.identity_no, uc->ext_addr, 16);
		incode_ctox16(&gw_info.panid, uc->panid);
		incode_ctox16(&gw_info.channel, uc->channel);
		get_frame_free(HEAD_UC, uc);
		break;
		
	case HEAD_UO:
		uo = (uo_t *)p;
		get_frame_free(HEAD_UO, uo);
		break;
		
	case HEAD_UH:
		uh = (uh_t *)p;
		get_frame_free(HEAD_UH, uh);
		break;
		
	case HEAD_UR:
		ur = (ur_t *)p;
		get_frame_free(HEAD_UR, ur);
		break;
		
	case HEAD_DE:
		de = (de_t *)p;
		get_frame_free(HEAD_DE, de);
		break;
		
	default:
		break;
	}
}
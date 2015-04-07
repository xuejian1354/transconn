/*
 * trframelysis.c
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

#include "trframelysis.h"

//traversal frame head
#define TR_HEAD_PI	"PI:"	//gw put info to server
#define TR_HEAD_BI	"BI:"	//server back info to gw
#define TR_HEAD_GP	"GP:"	//capps get ip port from server
#define TR_HEAD_RP	"RP:"	//server return ip port to capps & gw
#define TR_HEAD_GD	"GD:"	//get traversal connection
#define TR_HEAD_RD	"RD:"	//return traversal connection
#define TR_HEAD_DC	"DC:"	//down control
#define TR_HEAD_UB	"UB:"	//up back

#define TR_PI_DATA_FIX_LEN	15
#define TR_BI_DATA_FIX_LEN	8
#define TR_GP_DATA_FIX_LEN	31
#define TR_RP_DATA_FIX_LEN	34
#define TR_GD_DATA_FIX_LEN	7
#define TR_RD_DATA_FIX_LEN	7
#define TR_DC_DATA_FIX_LEN	7
#define TR_UB_DATA_FIX_LEN	7

#define TR_BUFFER_SIZE 	128
#define TR_TAIL ":O\r\n"

tr_head_type_t get_trhead_from_str(char *head)
{
	if(!strncmp(TR_HEAD_PI, head, 3))
	{
		return TRHEAD_PI;
	}
	else if(!strncmp(TR_HEAD_BI, head, 3))
	{
		return TRHEAD_BI;
	}
	else if(!strncmp(TR_HEAD_GP, head, 3))
	{
		return TRHEAD_GP;
	}
	else if(!strncmp(TR_HEAD_RP, head, 3))
	{
		return TRHEAD_RP;
	}
	else if(!strncmp(TR_HEAD_GD, head, 2))
	{
		return TRHEAD_GD;
	}
	else if(!strncmp(TR_HEAD_RD, head, 3))
	{
		return TRHEAD_RD;
	}
	else if(!strncmp(TR_HEAD_DC, head, 3))
	{
		return TRHEAD_DC;
	}
	else if(!strncmp(TR_HEAD_UB, head, 2))
	{
		return TRHEAD_UB;
	}
	
	return TRHEAD_NONE;
}


void *get_trframe_alloc(tr_head_type_t head_type, uint8 buffer[], int length)
{ 
	if(length > TR_BUFFER_SIZE)
	{
		goto tr_analysis_err;
	}
	
 	switch(head_type)
	{
	case TRHEAD_PI: 
		if(length>=TR_PI_DATA_FIX_LEN && !memcmp(buffer, TR_HEAD_PI, 3)
			&& !memcmp(buffer+TR_PI_DATA_FIX_LEN-4, TR_TAIL, 4))
		{
			pi_t *pi = (pi_t *)calloc(1, sizeof(pi_t));
			memcpy(pi->head, buffer, 3);
			memcpy(pi->zidentify_no, buffer+3, sizeof(zidentify_no_t));
			memcpy(pi->tail, buffer+TR_PI_DATA_FIX_LEN-4, 4);

			return (void *)pi;
		}
		else { goto  tr_analysis_err;}
		
	case TRHEAD_BI:
		if(length>=TR_BI_DATA_FIX_LEN && !memcmp(buffer, TR_HEAD_BI, 3)
			&& !memcmp(buffer+TR_PI_DATA_FIX_LEN-4, TR_TAIL, 4))
		{
			bi_t *bi = (bi_t *)calloc(1, sizeof(bi_t));
			memcpy(bi->head, buffer, 3);
			bi->trans_type = buffer[3];
			memcpy(bi->tail, buffer+TR_PI_DATA_FIX_LEN-4, 4);

			return (void *)bi;
		}
		else { goto  tr_analysis_err;}
		
	case TRHEAD_GP:
		if(length>=TR_GP_DATA_FIX_LEN && !memcmp(buffer, TR_HEAD_GP, 3)
			&& !memcmp(buffer+TR_GP_DATA_FIX_LEN-4, TR_TAIL, 4))
		{
			gp_t *gp = (gp_t *)calloc(1, sizeof(gp_t));
			memcpy(gp->head, buffer, 3);
			memcpy(gp->zidentify_no, buffer+3, sizeof(zidentify_no_t));
			memcpy(gp->cidentify_no, 
				buffer+3+sizeof(zidentify_no_t), sizeof(cidentify_no_t));
			memcpy(gp->tail, buffer+TR_PI_DATA_FIX_LEN-4, 4);

			return (void *)gp;
		}
		else { goto  tr_analysis_err;}
		
	case TRHEAD_RP:
		if(length>=TR_RP_DATA_FIX_LEN && !memcmp(buffer, TR_HEAD_RP, 3)
			&& !memcmp(buffer+length-4, TR_TAIL, 4))
		{
			rp_t *rp = (rp_t *)calloc(1, sizeof(rp_t));
			memcpy(rp->head, buffer, 3);
			memcpy(rp->zidentify_no, buffer+3, sizeof(zidentify_no_t));
			memcpy(rp->cidentify_no, 
				buffer+3+sizeof(zidentify_no_t), sizeof(cidentify_no_t));
			rp->trans_type = buffer[27];
			memcpy(rp->ipaddr, buffer+28, length-TR_RP_DATA_FIX_LEN);
			memcpy(rp->port, buffer+length-6, 2);
			memcpy(rp->tail, buffer+length-4, 4);

			return (void *)rp;
		}
		else { goto  tr_analysis_err;}
		
	case TRHEAD_GD:
		if(length>=TR_GD_DATA_FIX_LEN && !memcmp(buffer, TR_HEAD_GD, 3)
			&& !memcmp(buffer+TR_GD_DATA_FIX_LEN-4, TR_TAIL, 4))
		{
			gd_t *gd = (gd_t *)calloc(1, sizeof(gd_t));
			memcpy(gd->head, buffer, 3);
			memcpy(gd->tail, buffer+3, 4);

			return (void *)gd;
		}
		else { goto  tr_analysis_err;}
		
	case TRHEAD_RD:
		if(length>=TR_RD_DATA_FIX_LEN && !memcmp(buffer, TR_HEAD_RD, 3)
			&& !memcmp(buffer+TR_RD_DATA_FIX_LEN-4, TR_TAIL, 4))
		{
			rd_t *rd = (rd_t *)calloc(1, sizeof(rd_t));
			memcpy(rd->head, buffer, 3);
			memcpy(rd->tail, buffer+3, 4);

			return (void *)rd;
		}
		else { goto  tr_analysis_err;}
		
	case TRHEAD_DC:
		if(length>=TR_DC_DATA_FIX_LEN && !memcmp(buffer, TR_HEAD_DC, 3)
			&& !memcmp(buffer+TR_DC_DATA_FIX_LEN-4, TR_TAIL, 4))
		{
			dc_t *dc = (dc_t *)calloc(1, sizeof(dc_t));
			memcpy(dc->head, buffer, 3);
			memcpy(dc->tail, buffer+3, 4);

			if(length-TR_DC_DATA_FIX_LEN > 0)
			{
				uint8 *data_buffer = (uint8 *)calloc(length-TR_DC_DATA_FIX_LEN, sizeof(uint8));
				memcpy(data_buffer, buffer+3, length-TR_DC_DATA_FIX_LEN);
				dc->datalen = length-TR_DC_DATA_FIX_LEN;
				dc->data = data_buffer;
			}
			else
			{
				dc->datalen = 0;
				dc->data = NULL;
			}

			return (void *)dc;
		}
		else { goto  tr_analysis_err;}
		
	case TRHEAD_UB:
		if(length>=TR_UB_DATA_FIX_LEN && !memcmp(buffer, TR_HEAD_UB, 3)
			&& !memcmp(buffer+TR_UB_DATA_FIX_LEN-4, TR_TAIL, 4))
		{
			ub_t *ub = (ub_t *)calloc(1, sizeof(ub_t));
			memcpy(ub->head, buffer, 3);
			memcpy(ub->tail, buffer+3, 4);

			if(length-TR_UB_DATA_FIX_LEN > 0)
			{
				uint8 *data_buffer = (uint8 *)calloc(length-TR_UB_DATA_FIX_LEN, sizeof(uint8));
				memcpy(data_buffer, buffer+3, length-TR_UB_DATA_FIX_LEN);
				ub->datalen = length-TR_UB_DATA_FIX_LEN;
				ub->data = data_buffer;
			}
			else
			{
				ub->datalen = 0;
				ub->data = NULL;
			}

			return (void *)ub;
		}
		else { goto  tr_analysis_err;}

	default: goto  tr_analysis_err;
	}

tr_analysis_err:
	return NULL;
}
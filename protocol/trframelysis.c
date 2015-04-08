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
			memcpy(pi->gw_no, buffer+3, 16);
			memcpy(pi->tail, buffer+19, 4);

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
			memcpy(rp->port, buffer+length-8, 4);
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

void get_trframe_free(tr_head_type_t head_type, void *p)
{
	switch(head_type)
	{
	case TRHEAD_PI: 
	case TRHEAD_BI:
	case TRHEAD_GP:
	case TRHEAD_RP:
	case TRHEAD_GD:
	case TRHEAD_RD:
		free(p);
		break;
		
	case TRHEAD_DC:
		free(((dc_t *)p)->data);
		free(p);
		break;
	case TRHEAD_UB:
		free(((ub_t *)p)->data);
		free(p);
		break;
		
	default: break;
	}
}


tr_buffer_t *get_trbuffer_alloc(tr_head_type_t type, void *frame)
{
	pi_t *p_pi; bi_t *p_bi; gp_t *p_gp; rp_t *p_rp;
	gd_t *p_gd; rd_t *p_rd; dc_t *p_dc; ub_t *p_ub;
	
	tr_buffer_t *frame_buffer;
	
	if(frame == NULL)
		goto  tr_package_err;
	
	switch(type)
	{
	case TRHEAD_PI: 
		p_pi = (pi_t *)frame;
		frame_buffer = (tr_buffer_t *)calloc(1, sizeof(tr_buffer_t));
		frame_buffer->size = TR_PI_DATA_FIX_LEN;
		frame_buffer->data = (uint8 *)calloc(frame_buffer->size, sizeof(uint8));
		
		memcpy(frame_buffer->data, p_pi->head, 3);
		memcpy(frame_buffer->data+3, p_pi->gw_no, 16);
		memcpy(frame_buffer->data+19, p_pi->tail, 4);

		return frame_buffer;
		
	case TRHEAD_BI:
		p_bi = (bi_t *)frame;
		frame_buffer = (tr_buffer_t *)calloc(1, sizeof(tr_buffer_t));
		frame_buffer->size = TR_BI_DATA_FIX_LEN;
		frame_buffer->data = (uint8 *)calloc(frame_buffer->size, sizeof(uint8));
		
		memcpy(frame_buffer->data, p_bi->head, 3);
		frame_buffer->data[3] = p_bi->trans_type;
		memcpy(frame_buffer->data+4, p_bi->tail, 4);

		return frame_buffer;
		
	case TRHEAD_GP:
		p_gp = (gp_t *)frame;
		frame_buffer = (tr_buffer_t *)calloc(1, sizeof(tr_buffer_t));
		frame_buffer->size = TR_GP_DATA_FIX_LEN;
		frame_buffer->data = (uint8 *)calloc(frame_buffer->size, sizeof(uint8));
		
		memcpy(frame_buffer->data, p_gp->head, 3);
		memcpy(frame_buffer->data+3, p_gp->zidentify_no, sizeof(zidentify_no_t));
		memcpy(frame_buffer->data+11, p_gp->cidentify_no, sizeof(cidentify_no_t));
		memcpy(frame_buffer->data+27, p_gp->tail, 4);

		return frame_buffer;
		
	case TRHEAD_RP:
		p_rp = (rp_t *)frame;
		frame_buffer = (tr_buffer_t *)calloc(1, sizeof(tr_buffer_t));
		frame_buffer->size = TR_RP_DATA_FIX_LEN+strlen(p_rp->ipaddr);
		if(frame_buffer->size > TR_BUFFER_SIZE)
		{
			free(frame_buffer);
			return NULL;
		}
		frame_buffer->data = (uint8 *)calloc(frame_buffer->size, sizeof(uint8));
		
		memcpy(frame_buffer->data, p_rp->head, 3);
		memcpy(frame_buffer->data+3, p_rp->zidentify_no, sizeof(zidentify_no_t));
		memcpy(frame_buffer->data+11, p_rp->cidentify_no, sizeof(cidentify_no_t));
		frame_buffer->data[27] = p_rp->trans_type;
		memcpy(frame_buffer->data+28, p_rp->ipaddr, strlen(p_rp->ipaddr));
		memcpy(frame_buffer->data+frame_buffer->size-8, p_rp->port, 4);
		memcpy(frame_buffer->data+frame_buffer->size-4, p_rp->tail, 4);

		return frame_buffer;
		
	case TRHEAD_GD:
		p_gd = (gd_t *)frame;
		frame_buffer = (tr_buffer_t *)calloc(1, sizeof(tr_buffer_t));
		frame_buffer->size = TR_GD_DATA_FIX_LEN;
		frame_buffer->data = (uint8 *)calloc(frame_buffer->size, sizeof(uint8));
		
		memcpy(frame_buffer->data, p_gd->head, 3);
		memcpy(frame_buffer->data+3, p_gd->tail, 4);

		return frame_buffer;
		
	case TRHEAD_RD:
		p_rd = (rd_t *)frame;
		frame_buffer = (tr_buffer_t *)calloc(1, sizeof(tr_buffer_t));
		frame_buffer->size = TR_RD_DATA_FIX_LEN;
		frame_buffer->data = (uint8 *)calloc(frame_buffer->size, sizeof(uint8));
		
		memcpy(frame_buffer->data, p_rd->head, 3);
		memcpy(frame_buffer->data+3, p_rd->tail, 4);

		return frame_buffer;
		
	case TRHEAD_DC:
		p_dc = (dc_t *)frame;
		frame_buffer = (tr_buffer_t *)calloc(1, sizeof(tr_buffer_t));
		frame_buffer->size = TR_DC_DATA_FIX_LEN+p_dc->datalen;
		if(frame_buffer->size > TR_BUFFER_SIZE)
		{
			free(frame_buffer);
			return NULL;
		}
		frame_buffer->data = (uint8 *)calloc(frame_buffer->size, sizeof(uint8));
		
		memcpy(frame_buffer->data, p_dc->head, 3);
		memcpy(frame_buffer->data+3, p_dc->data, p_dc->datalen);
		memcpy(frame_buffer->data+frame_buffer->size-4, p_dc->tail, 4);

		return frame_buffer;
		
	case TRHEAD_UB:
		p_ub = (ub_t *)frame;
		frame_buffer = (tr_buffer_t *)calloc(1, sizeof(tr_buffer_t));
		frame_buffer->size = TR_UB_DATA_FIX_LEN+p_ub->datalen;
		if(frame_buffer->size > TR_BUFFER_SIZE)
		{
			free(frame_buffer);
			return NULL;
		}
		frame_buffer->data = (uint8 *)calloc(frame_buffer->size, sizeof(uint8));
		
		memcpy(frame_buffer->data, p_ub->head, 3);
		memcpy(frame_buffer->data+3, p_ub->data, p_ub->datalen);
		memcpy(frame_buffer->data+frame_buffer->size-4, p_ub->tail, 4);

		return frame_buffer;
	
	default: goto  tr_package_err;
	}

tr_package_err:
	return NULL;
}


void get_trbuffer_free(tr_buffer_t *p)
{
	free(p->data);
	free(p);
}
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

tr_trans_type_t get_trtrans_from_ch(char trans)
{
	switch(trans)
	{
	case TR_TYPE_UDP:
		return TRTYPE_UDP;

	case TR_TYPE_TCP:
		return TRTYPE_TCP;

	default:
		break;
	}

	return TRTYPE_NONE;
}

char get_trtrans_to_ch(tr_trans_type_t trans)
{
	switch(trans)
	{
	case TRTYPE_UDP:
		return TR_TYPE_UDP;

	case TRTYPE_TCP:
		return TR_TYPE_TCP;

	default:
		break;
	}

	return '0';
}

tr_frame_type_t get_trframe_from_ch(char trfra)
{
	switch(trfra)
	{
	case TR_FRAME_CON:
		return TRFRAME_CON;

	case TR_FRAME_REG:
		return TRFRAME_REG;

	case TR_FRAME_GET:
		return TRFRAME_GET;

	case TR_FRAME_PUT_GW:
		return TRFRAME_PUT_GW;

	case TR_FRAME_PUT_DEV:
		return TRFRAME_PUT_DEV;

	default:
		break;
	}

	return TRFRAME_NONE;
}

char get_trframe_to_ch(tr_frame_type_t trfra)
{
	switch(trfra)
	{
	case TRFRAME_CON:
		return TR_FRAME_CON;

	case TRFRAME_REG:
		return TR_FRAME_REG;

	case TRFRAME_GET:
		return TR_FRAME_GET;

	case TRFRAME_PUT_GW:
		return TR_FRAME_PUT_GW;

	case TRFRAME_PUT_DEV:
		return TR_FRAME_PUT_DEV;

	default:
		break;
	}

	return '0';
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
			&& !memcmp(buffer+length-4, TR_TAIL, 4))
		{
			pi_t *pi = (pi_t *)calloc(1, sizeof(pi_t));
			incode_ctoxs(pi->sn, buffer+3, 16);
			pi->trans_type = get_trtrans_from_ch(buffer[19]);
			pi->fr_type = get_trframe_from_ch(buffer[20]);
			
			if(length-TR_PI_DATA_FIX_LEN > 0)
			{
				pi->data_len = length-TR_PI_DATA_FIX_LEN;
				uint8 *data_buffer = (uint8 *)calloc(pi->data_len, sizeof(uint8));
				memcpy(data_buffer, buffer+21, pi->data_len);
				pi->data = data_buffer;
			}
			else
			{
				pi->data_len = 0;
				pi->data = NULL;
			}

			return (void *)pi;
		}
		else { goto  tr_analysis_err;}
		
	case TRHEAD_BI:
		if(length>=TR_BI_DATA_FIX_LEN && !memcmp(buffer, TR_HEAD_BI, 3)
			&& !memcmp(buffer+length-4, TR_TAIL, 4))
		{
			bi_t *bi = (bi_t *)calloc(1, sizeof(bi_t));
			incode_ctoxs(bi->sn, buffer+3, 16);
			bi->trans_type = get_trtrans_from_ch(buffer[19]);
			bi->fr_type = get_trframe_from_ch(buffer[20]);
			
			if(length-TR_BI_DATA_FIX_LEN > 0)
			{
				bi->data_len = length-TR_BI_DATA_FIX_LEN;
				uint8 *data_buffer = (uint8 *)calloc(bi->data_len, sizeof(uint8));
				memcpy(data_buffer, buffer+21, bi->data_len);
				bi->data = data_buffer;
			}
			else
			{
				bi->data_len = 0;
				bi->data = NULL;
			}

			return (void *)bi;
		}
		else { goto  tr_analysis_err;}
		
	case TRHEAD_GP:
		if(length>=TR_GP_DATA_FIX_LEN && !memcmp(buffer, TR_HEAD_GP, 3)
			&& !memcmp(buffer+length-4, TR_TAIL, 4))
		{
			gp_t *gp = (gp_t *)calloc(1, sizeof(gp_t));
			incode_ctoxs(gp->zidentify_no, buffer+3, 16);
			incode_ctoxs(gp->zidentify_no, buffer+19, 16);
			if(length-TR_GP_DATA_FIX_LEN > 0)
			{
				gp->data_len = length-TR_GP_DATA_FIX_LEN;
				uint8 *data_buffer = (uint8 *)calloc(gp->data_len, sizeof(uint8));
				memcpy(data_buffer, buffer+35, gp->data_len);
				gp->data = data_buffer;
			}
			else
			{
				gp->data_len = 0;
				gp->data = NULL;
			}

			return (void *)gp;
		}
		else { goto  tr_analysis_err;}
		
	case TRHEAD_RP:
		if(length>=TR_RP_DATA_FIX_LEN && !memcmp(buffer, TR_HEAD_RP, 3)
			&& !memcmp(buffer+length-4, TR_TAIL, 4))
		{
			rp_t *rp = (rp_t *)calloc(1, sizeof(rp_t));
			incode_ctoxs(rp->zidentify_no, buffer+3, 16);
			incode_ctoxs(rp->zidentify_no, buffer+19, 16);
			if(length-TR_RP_DATA_FIX_LEN > 0)
			{
				rp->data_len = length-TR_RP_DATA_FIX_LEN;
				uint8 *data_buffer = (uint8 *)calloc(rp->data_len, sizeof(uint8));
				memcpy(data_buffer, buffer+35, rp->data_len);
				rp->data = data_buffer;
			}
			else
			{
				rp->data_len = 0;
				rp->data = NULL;
			}

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
		free(((pi_t *)p)->data);
		free(p);
		break;
		
	case TRHEAD_BI:
		free(((bi_t *)p)->data);
		free(p);
		break;
		
	case TRHEAD_GP:
		free(((gp_t *)p)->data);
		free(p);
		break;
		
	case TRHEAD_RP:
		free(((rp_t *)p)->data);
		free(p);
		break;
		
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
		frame_buffer->size = TR_PI_DATA_FIX_LEN+p_pi->data_len;
		if(frame_buffer->size > TR_BUFFER_SIZE)
		{
			free(frame_buffer);
			goto tr_package_err;
		}
		frame_buffer->data = (uint8 *)calloc(frame_buffer->size, sizeof(uint8));
		
		memcpy(frame_buffer->data, TR_HEAD_PI, 3);
		incode_xtocs(frame_buffer->data+3, p_pi->sn, 8);
		frame_buffer->data[19] = get_trtrans_to_ch(p_pi->trans_type);
		frame_buffer->data[20] = get_trframe_to_ch(p_pi->fr_type);
		memcpy(frame_buffer->data+21, p_pi->data, p_pi->data_len);
		memcpy(frame_buffer->data+frame_buffer->size-4, TR_TAIL, 4);

		return frame_buffer;
		
	case TRHEAD_BI:
		p_bi = (bi_t *)frame;
		frame_buffer = (tr_buffer_t *)calloc(1, sizeof(tr_buffer_t));
		frame_buffer->size = TR_BI_DATA_FIX_LEN+p_bi->data_len;
		if(frame_buffer->size > TR_BUFFER_SIZE)
		{
			free(frame_buffer);
			goto tr_package_err;
		}
		frame_buffer->data = (uint8 *)calloc(frame_buffer->size, sizeof(uint8));
		
		memcpy(frame_buffer->data, TR_HEAD_BI, 3);
		incode_xtocs(frame_buffer->data+3, p_bi->sn, 8);
		frame_buffer->data[19] = get_trtrans_to_ch(p_bi->trans_type);
		frame_buffer->data[20] = get_trframe_to_ch(p_bi->fr_type);
		memcpy(frame_buffer->data+21, p_bi->data, p_bi->data_len);
		memcpy(frame_buffer->data+frame_buffer->size-4, TR_TAIL, 4);

		return frame_buffer;
		
	case TRHEAD_GP:
		p_gp = (gp_t *)frame;
		frame_buffer = (tr_buffer_t *)calloc(1, sizeof(tr_buffer_t));
		frame_buffer->size = TR_GP_DATA_FIX_LEN+p_gp->data_len;
		if(frame_buffer->size > TR_BUFFER_SIZE)
		{
			free(frame_buffer);
			goto tr_package_err;
		}
		frame_buffer->data = (uint8 *)calloc(frame_buffer->size, sizeof(uint8));
		
		memcpy(frame_buffer->data, TR_HEAD_GP, 3);
		incode_xtocs(frame_buffer->data+3, p_gp->zidentify_no, 8);
		incode_xtocs(frame_buffer->data+19, p_gp->cidentify_no, 8);
		memcpy(frame_buffer->data+35, p_gp->data, p_gp->data_len);
		memcpy(frame_buffer->data+frame_buffer->size-4, TR_TAIL, 4);

		return frame_buffer;
		
	case TRHEAD_RP:
		p_rp = (rp_t *)frame;
		frame_buffer = (tr_buffer_t *)calloc(1, sizeof(tr_buffer_t));
		frame_buffer->size = TR_RP_DATA_FIX_LEN+p_rp->data_len;
		if(frame_buffer->size > TR_BUFFER_SIZE)
		{
			free(frame_buffer);
			goto tr_package_err;
		}
		frame_buffer->data = (uint8 *)calloc(frame_buffer->size, sizeof(uint8));
		
		memcpy(frame_buffer->data, TR_HEAD_RP, 3);
		incode_xtocs(frame_buffer->data+3, p_rp->zidentify_no, 8);
		incode_xtocs(frame_buffer->data+19, p_rp->cidentify_no, 8);
		memcpy(frame_buffer->data+35, p_rp->data, p_rp->data_len);
		memcpy(frame_buffer->data+frame_buffer->size-4, TR_TAIL, 4);

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
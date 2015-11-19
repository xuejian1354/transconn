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
	else if(!strncmp(TR_HEAD_UL, head, 3))
	{
		return TRHEAD_UL;
	}
	else if(!strncmp(TR_HEAD_SL, head, 3))
	{
		return TRHEAD_SL;
	}
	else if(!strncmp(TR_HEAD_UT, head, 3))
	{
		return TRHEAD_UT;
	}
	else if(!strncmp(TR_HEAD_ST, head, 3))
	{
		return TRHEAD_ST;
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
	case TR_TYPE_UDP_NORMAL:
		return TRTYPE_UDP_NORMAL;

	case TR_TYPE_UDP_TRAVERSAL:
		return TRTYPE_UDP_TRAVERSAL;

	case TR_TYPE_UDP_TRANS:
		return TRTYPE_UDP_TRANS;

	case TR_TYPE_TCP_LONG:
		return TRTYPE_TCP_LONG;

	case TR_TYPE_TCP_SHORT:
		return TRTYPE_TCP_SHORT;

	default:
		break;
	}

	return TRTYPE_NONE;
}

char get_trtrans_to_ch(tr_trans_type_t trans)
{
	switch(trans)
	{
	case TRTYPE_UDP_NORMAL:
		return TR_TYPE_UDP_NORMAL;

	case TRTYPE_UDP_TRAVERSAL:
		return TR_TYPE_UDP_TRAVERSAL;

	case TRTYPE_UDP_TRANS:
		return TR_TYPE_UDP_TRANS;

	case TRTYPE_TCP_LONG:
		return TR_TYPE_TCP_LONG;

	case TRTYPE_TCP_SHORT:
		return TR_TYPE_TCP_SHORT;

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

	case TR_FRAME_TRANS:
		return TRFRAME_TRANS;

	case TR_FRAME_IGNORE:
		return TRFRAME_IGNORE;

	case TR_FRAME_SYNC_CONF:
		return TRFRAME_SYNC_CONF;

	case TR_FRAME_DEL_CONF:
		return TRFRAME_DEL_CONF;

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

	case TRFRAME_TRANS:
		return TR_FRAME_TRANS;

	case TRFRAME_IGNORE:
		return TR_FRAME_IGNORE;

	case TRFRAME_SYNC_CONF:
		return TR_FRAME_SYNC_CONF;

	case TRFRAME_DEL_CONF:
		return TR_FRAME_DEL_CONF;

	default:
		break;
	}

	return '0';
}

tr_info_type_t get_trinfo_from_ch(char trinfo)
{
	switch(trinfo)
	{
	case TR_INFO_IP:
		return TRINFO_IP;

	case TR_INFO_DATA:
		return TRINFO_DATA;

	case TR_INFO_UPDATE:
		return TRINFO_UPDATE;

	case TR_INFO_CUT:
		return TRINFO_CUT;

	case TR_INFO_REG:
		return TRINFO_REG;

	case TR_INFO_HOLD:
		return TRINFO_HOLD;

	case TR_INFO_FOUND:
		return TRINFO_FOUND;

	case TR_INFO_DISMATCH:
		return TRINFO_DISMATCH;

	case TR_INFO_CONTROL:
		return TRINFO_CONTROL;

	case TR_INFO_QUERY:
		return TRINFO_QUERY;

	case TR_INFO_REDATA:
		return TRINFO_REDATA;

	default:
		break;
	}

	return TRINFO_NONE;
}

char get_trinfo_to_ch(tr_info_type_t trinfo)
{
	switch(trinfo)
	{
	case TRINFO_IP:
		return TR_INFO_IP;

	case TRINFO_DATA:
		return TR_INFO_DATA;

	case TRINFO_UPDATE:
		return TR_INFO_UPDATE;

	case TRINFO_CUT:
		return TR_INFO_CUT;

	case TRINFO_REG:
		return TR_INFO_REG;

	case TRINFO_HOLD:
		return TR_INFO_HOLD;

	case TRINFO_FOUND:
		return TR_INFO_FOUND;

	case TRINFO_DISMATCH:
		return TR_INFO_DISMATCH;

	case TRINFO_CONTROL:
		return TR_INFO_CONTROL;

	case TRINFO_QUERY:
		return TR_INFO_QUERY;

	case TRINFO_REDATA:
		return TR_INFO_REDATA;

	default:
		break;
	}

	return TR_INFO_NONE;
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

	case TRHEAD_UL:
		if(length>=TR_UL_DATA_FIX_LEN && !memcmp(buffer, TR_HEAD_UL, 3)
			&& !memcmp(buffer+length-4, TR_TAIL, 4))
		{
			ul_t *ul = (ul_t *)calloc(1, sizeof(ul_t));
			incode_ctoxs(ul->sn, buffer+3, 16);
			ul->trans_type = get_trtrans_from_ch(buffer[19]);
			ul->fr_type = get_trframe_from_ch(buffer[20]);
			
			if(length-TR_UL_DATA_FIX_LEN > 0)
			{
				ul->data_len = length-TR_UL_DATA_FIX_LEN;
				uint8 *data_buffer = (uint8 *)calloc(ul->data_len, sizeof(uint8));
				memcpy(data_buffer, buffer+21, ul->data_len);
				ul->data = data_buffer;
			}
			else
			{
				ul->data_len = 0;
				ul->data = NULL;
			}

			return (void *)ul;
		}
		else { goto  tr_analysis_err;}

	case TRHEAD_SL:
		if(length>=TR_SL_DATA_FIX_LEN && !memcmp(buffer, TR_HEAD_SL, 3)
			&& !memcmp(buffer+length-4, TR_TAIL, 4))
		{
			sl_t *sl = (sl_t *)calloc(1, sizeof(sl_t));
			incode_ctoxs(sl->sn, buffer+3, 16);
			sl->trans_type = get_trtrans_from_ch(buffer[19]);
			sl->fr_type = get_trframe_from_ch(buffer[20]);
			
			if(length-TR_SL_DATA_FIX_LEN > 0)
			{
				sl->data_len = length-TR_SL_DATA_FIX_LEN;
				uint8 *data_buffer = (uint8 *)calloc(sl->data_len, sizeof(uint8));
				memcpy(data_buffer, buffer+21, sl->data_len);
				sl->data = data_buffer;
			}
			else
			{
				sl->data_len = 0;
				sl->data = NULL;
			}

			return (void *)sl;
		}
		else { goto  tr_analysis_err;}

	case TRHEAD_UT:
		if(length>=TR_UT_DATA_FIX_LEN && !memcmp(buffer, TR_HEAD_UT, 3)
			&& !memcmp(buffer+length-4, TR_TAIL, 4))
		{
			ut_t *ut = (ut_t *)calloc(1, sizeof(ut_t));
			incode_ctoxs(ut->sn, buffer+3, 16);
			ut->trans_type = get_trtrans_from_ch(buffer[19]);
			ut->fr_type = get_trframe_from_ch(buffer[20]);
			
			if(length-TR_UT_DATA_FIX_LEN > 0)
			{
				ut->data_len = length-TR_UT_DATA_FIX_LEN;
				uint8 *data_buffer = (uint8 *)calloc(ut->data_len, sizeof(uint8));
				memcpy(data_buffer, buffer+21, ut->data_len);
				ut->data = data_buffer;
			}
			else
			{
				ut->data_len = 0;
				ut->data = NULL;
			}

			return (void *)ut;
		}
		else { goto  tr_analysis_err;}

	case TRHEAD_ST:
		if(length>=TR_ST_DATA_FIX_LEN && !memcmp(buffer, TR_HEAD_ST, 3)
			&& !memcmp(buffer+length-4, TR_TAIL, 4))
		{
			st_t *st = (st_t *)calloc(1, sizeof(st_t));
			incode_ctoxs(st->sn, buffer+3, 16);
			st->trans_type = get_trtrans_from_ch(buffer[19]);
			st->fr_type = get_trframe_from_ch(buffer[20]);
			
			if(length-TR_ST_DATA_FIX_LEN > 0)
			{
				st->data_len = length-TR_ST_DATA_FIX_LEN;
				uint8 *data_buffer = (uint8 *)calloc(st->data_len, sizeof(uint8));
				memcpy(data_buffer, buffer+21, st->data_len);
				st->data = data_buffer;
			}
			else
			{
				st->data_len = 0;
				st->data = NULL;
			}

			return (void *)st;
		}
		else { goto  tr_analysis_err;}
		
	case TRHEAD_GP:
		if(length>=TR_GP_DATA_FIX_LEN && !memcmp(buffer, TR_HEAD_GP, 3)
			&& !memcmp(buffer+length-4, TR_TAIL, 4))
		{
			gp_t *gp = (gp_t *)calloc(1, sizeof(gp_t));
			incode_ctoxs(gp->zidentify_no, buffer+3, 16);
			incode_ctoxs(gp->cidentify_no, buffer+19, 16);
			gp->trans_type = get_trtrans_from_ch(buffer[35]);
			gp->tr_info = get_trinfo_from_ch(buffer[36]);
			if(length-TR_GP_DATA_FIX_LEN > 0)
			{
				gp->data_len = length-TR_GP_DATA_FIX_LEN;
				uint8 *data_buffer = (uint8 *)calloc(gp->data_len, sizeof(uint8));
				memcpy(data_buffer, buffer+37, gp->data_len);
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
			incode_ctoxs(rp->cidentify_no, buffer+19, 16);
			rp->trans_type = get_trtrans_from_ch(buffer[35]);
			rp->tr_info = get_trinfo_from_ch(buffer[36]);
			if(length-TR_RP_DATA_FIX_LEN > 0)
			{
				rp->data_len = length-TR_RP_DATA_FIX_LEN;
				uint8 *data_buffer = (uint8 *)calloc(rp->data_len, sizeof(uint8));
				memcpy(data_buffer, buffer+37, rp->data_len);
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
			&& !memcmp(buffer+length-4, TR_TAIL, 4))
		{
			gd_t *gd = (gd_t *)calloc(1, sizeof(gd_t));
			incode_ctoxs(gd->zidentify_no, buffer+3, 16);
			incode_ctoxs(gd->cidentify_no, buffer+19, 16);
			gd->trans_type = get_trtrans_from_ch(buffer[35]);
			gd->tr_info = get_trinfo_from_ch(buffer[36]);
			if(length-TR_GD_DATA_FIX_LEN > 0)
			{
				gd->data_len = length-TR_GD_DATA_FIX_LEN;
				uint8 *data_buffer = (uint8 *)calloc(gd->data_len, sizeof(uint8));
				memcpy(data_buffer, buffer+37, gd->data_len);
				gd->data = data_buffer;
			}
			else
			{
				gd->data_len = 0;
				gd->data = NULL;
			}

			return (void *)gd;
		}
		else { goto  tr_analysis_err;}
		
	case TRHEAD_RD:
		if(length>=TR_RD_DATA_FIX_LEN && !memcmp(buffer, TR_HEAD_RD, 3)
			&& !memcmp(buffer+length-4, TR_TAIL, 4))
		{
			rd_t *rd = (rd_t *)calloc(1, sizeof(rd_t));
			incode_ctoxs(rd->zidentify_no, buffer+3, 16);
			incode_ctoxs(rd->cidentify_no, buffer+19, 16);
			rd->trans_type = get_trtrans_from_ch(buffer[35]);
			rd->tr_info = get_trinfo_from_ch(buffer[36]);
			if(length-TR_RD_DATA_FIX_LEN > 0)
			{
				rd->data_len = length-TR_RD_DATA_FIX_LEN;
				uint8 *data_buffer = (uint8 *)calloc(rd->data_len, sizeof(uint8));
				memcpy(data_buffer, buffer+37, rd->data_len);
				rd->data = data_buffer;
			}
			else
			{
				rd->data_len = 0;
				rd->data = NULL;
			}

			return (void *)rd;
		}
		else { goto  tr_analysis_err;}
		
	case TRHEAD_DC:
		if(length>=TR_DC_DATA_FIX_LEN && !memcmp(buffer, TR_HEAD_DC, 3)
			&& !memcmp(buffer+length-4, TR_TAIL, 4))
		{
			dc_t *dc = (dc_t *)calloc(1, sizeof(dc_t));
			incode_ctoxs(dc->zidentify_no, buffer+3, 16);
			incode_ctoxs(dc->cidentify_no, buffer+19, 16);
			dc->trans_type = get_trtrans_from_ch(buffer[35]);
			dc->tr_info = get_trinfo_from_ch(buffer[36]);
			if(length-TR_DC_DATA_FIX_LEN > 0)
			{
				uint8 *data_buffer = (uint8 *)calloc(length-TR_DC_DATA_FIX_LEN, sizeof(uint8));
				memcpy(data_buffer, buffer+37, length-TR_DC_DATA_FIX_LEN);
				dc->data_len = length-TR_DC_DATA_FIX_LEN;
				dc->data = data_buffer;
			}
			else
			{
				dc->data_len = 0;
				dc->data = NULL;
			}
			
			return (void *)dc;
		}
		else { goto  tr_analysis_err;}
		
	case TRHEAD_UB:
		if(length>=TR_UB_DATA_FIX_LEN && !memcmp(buffer, TR_HEAD_UB, 3)
			&& !memcmp(buffer+length-4, TR_TAIL, 4))
		{
			ub_t *ub = (ub_t *)calloc(1, sizeof(ub_t));
			incode_ctoxs(ub->zidentify_no, buffer+3, 16);
			incode_ctoxs(ub->cidentify_no, buffer+19, 16);
			ub->trans_type = get_trtrans_from_ch(buffer[35]);
			ub->tr_info = get_trinfo_from_ch(buffer[36]);
			if(length-TR_UB_DATA_FIX_LEN > 0)
			{
				uint8 *data_buffer = (uint8 *)calloc(length-TR_UB_DATA_FIX_LEN, sizeof(uint8));
				memcpy(data_buffer, buffer+37, length-TR_UB_DATA_FIX_LEN);
				ub->data_len = length-TR_UB_DATA_FIX_LEN;
				ub->data = data_buffer;
			}
			else
			{
				ub->data_len = 0;
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

	case TRHEAD_UL:
		free(((ul_t *)p)->data);
		free(p);
		break;

	case TRHEAD_SL:
		free(((sl_t *)p)->data);
		free(p);
		break;

	case TRHEAD_UT:
		free(((ut_t *)p)->data);
		free(p);
		break;

	case TRHEAD_ST:
		free(((st_t *)p)->data);
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
		free(((gd_t *)p)->data);
		free(p);
		break;
		
	case TRHEAD_RD:
		free(((rd_t *)p)->data);
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
	tr_buffer_t *frame_buffer;
	
	if(frame == NULL)
		goto  tr_package_err;
	
	switch(type)
	{
	case TRHEAD_PI: 
	{
		pi_t *p_pi = (pi_t *)frame;
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
	}
		
	case TRHEAD_BI:
	{
		bi_t *p_bi = (bi_t *)frame;
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
	}

	case TRHEAD_UL:
	{
		ul_t *p_ul = (ul_t *)frame;
		frame_buffer = (tr_buffer_t *)calloc(1, sizeof(tr_buffer_t));
		frame_buffer->size = TR_UL_DATA_FIX_LEN+p_ul->data_len;
		if(frame_buffer->size > TR_BUFFER_SIZE)
		{
			free(frame_buffer);
			goto tr_package_err;
		}
		frame_buffer->data = (uint8 *)calloc(frame_buffer->size, sizeof(uint8));
		
		memcpy(frame_buffer->data, TR_HEAD_UL, 3);
		incode_xtocs(frame_buffer->data+3, p_ul->sn, 8);
		frame_buffer->data[19] = get_trtrans_to_ch(p_ul->trans_type);
		frame_buffer->data[20] = get_trframe_to_ch(p_ul->fr_type);
		memcpy(frame_buffer->data+21, p_ul->data, p_ul->data_len);
		memcpy(frame_buffer->data+frame_buffer->size-4, TR_TAIL, 4);

		return frame_buffer;
	}

	case TRHEAD_SL:
	{
		sl_t *p_sl = (sl_t *)frame;
		frame_buffer = (tr_buffer_t *)calloc(1, sizeof(tr_buffer_t));
		frame_buffer->size = TR_SL_DATA_FIX_LEN+p_sl->data_len;
		if(frame_buffer->size > TR_BUFFER_SIZE)
		{
			free(frame_buffer);
			goto tr_package_err;
		}
		frame_buffer->data = (uint8 *)calloc(frame_buffer->size, sizeof(uint8));
		
		memcpy(frame_buffer->data, TR_HEAD_SL, 3);
		incode_xtocs(frame_buffer->data+3, p_sl->sn, 8);
		frame_buffer->data[19] = get_trtrans_to_ch(p_sl->trans_type);
		frame_buffer->data[20] = get_trframe_to_ch(p_sl->fr_type);
		memcpy(frame_buffer->data+21, p_sl->data, p_sl->data_len);
		memcpy(frame_buffer->data+frame_buffer->size-4, TR_TAIL, 4);

		return frame_buffer;
	}

	case TRHEAD_UT:
	{
		ut_t *p_ut = (ut_t *)frame;
		frame_buffer = (tr_buffer_t *)calloc(1, sizeof(tr_buffer_t));
		frame_buffer->size = TR_UT_DATA_FIX_LEN+p_ut->data_len;
		if(frame_buffer->size > TR_BUFFER_SIZE)
		{
			free(frame_buffer);
			goto tr_package_err;
		}
		frame_buffer->data = (uint8 *)calloc(frame_buffer->size, sizeof(uint8));
		
		memcpy(frame_buffer->data, TR_HEAD_UT, 3);
		incode_xtocs(frame_buffer->data+3, p_ut->sn, 8);
		frame_buffer->data[19] = get_trtrans_to_ch(p_ut->trans_type);
		frame_buffer->data[20] = get_trframe_to_ch(p_ut->fr_type);
		memcpy(frame_buffer->data+21, p_ut->data, p_ut->data_len);
		memcpy(frame_buffer->data+frame_buffer->size-4, TR_TAIL, 4);

		return frame_buffer;
	}

	case TRHEAD_ST:
	{
		st_t *p_st = (st_t *)frame;
		frame_buffer = (tr_buffer_t *)calloc(1, sizeof(tr_buffer_t));
		frame_buffer->size = TR_ST_DATA_FIX_LEN+p_st->data_len;
		if(frame_buffer->size > TR_BUFFER_SIZE)
		{
			free(frame_buffer);
			goto tr_package_err;
		}
		frame_buffer->data = (uint8 *)calloc(frame_buffer->size, sizeof(uint8));
		
		memcpy(frame_buffer->data, TR_HEAD_ST, 3);
		incode_xtocs(frame_buffer->data+3, p_st->sn, 8);
		frame_buffer->data[19] = get_trtrans_to_ch(p_st->trans_type);
		frame_buffer->data[20] = get_trframe_to_ch(p_st->fr_type);
		memcpy(frame_buffer->data+21, p_st->data, p_st->data_len);
		memcpy(frame_buffer->data+frame_buffer->size-4, TR_TAIL, 4);

		return frame_buffer;
	}
		
	case TRHEAD_GP:
	{
		gp_t *p_gp = (gp_t *)frame;
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
		frame_buffer->data[35] = get_trtrans_to_ch(p_gp->trans_type);
		frame_buffer->data[36] = get_trinfo_to_ch(p_gp->tr_info);
		memcpy(frame_buffer->data+37, p_gp->data, p_gp->data_len);
		memcpy(frame_buffer->data+frame_buffer->size-4, TR_TAIL, 4);

		return frame_buffer;
	}
		
	case TRHEAD_RP:
	{
		rp_t *p_rp = (rp_t *)frame;
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
		frame_buffer->data[35] = get_trtrans_to_ch(p_rp->trans_type);
		frame_buffer->data[36] = get_trinfo_to_ch(p_rp->tr_info);
		memcpy(frame_buffer->data+37, p_rp->data, p_rp->data_len);
		memcpy(frame_buffer->data+frame_buffer->size-4, TR_TAIL, 4);

		return frame_buffer;
	}
		
	case TRHEAD_GD:
	{
		gd_t *p_gd = (gd_t *)frame;
		frame_buffer = (tr_buffer_t *)calloc(1, sizeof(tr_buffer_t));
		frame_buffer->size = TR_GD_DATA_FIX_LEN+p_gd->data_len;
		if(frame_buffer->size > TR_BUFFER_SIZE)
		{
			free(frame_buffer);
			goto tr_package_err;
		}
		frame_buffer->data = (uint8 *)calloc(frame_buffer->size, sizeof(uint8));
		
		memcpy(frame_buffer->data, TR_HEAD_GD, 3);
		incode_xtocs(frame_buffer->data+3, p_gd->zidentify_no, 8);
		incode_xtocs(frame_buffer->data+19, p_gd->cidentify_no, 8);
		frame_buffer->data[35] = get_trtrans_to_ch(p_gd->trans_type);
		frame_buffer->data[36] = get_trinfo_to_ch(p_gd->tr_info);
		memcpy(frame_buffer->data+37, p_gd->data, p_gd->data_len);
		memcpy(frame_buffer->data+frame_buffer->size-4, TR_TAIL, 4);

		return frame_buffer;
	}
		
	case TRHEAD_RD:
	{
		rd_t *p_rd = (rd_t *)frame;
		frame_buffer = (tr_buffer_t *)calloc(1, sizeof(tr_buffer_t));
		frame_buffer->size = TR_RD_DATA_FIX_LEN+p_rd->data_len;
		if(frame_buffer->size > TR_BUFFER_SIZE)
		{
			free(frame_buffer);
			goto tr_package_err;
		}
		frame_buffer->data = (uint8 *)calloc(frame_buffer->size, sizeof(uint8));
		
		memcpy(frame_buffer->data, TR_HEAD_RD, 3);
		incode_xtocs(frame_buffer->data+3, p_rd->zidentify_no, 8);
		incode_xtocs(frame_buffer->data+19, p_rd->cidentify_no, 8);
		frame_buffer->data[35] = get_trtrans_to_ch(p_rd->trans_type);
		frame_buffer->data[36] = get_trinfo_to_ch(p_rd->tr_info);
		memcpy(frame_buffer->data+37, p_rd->data, p_rd->data_len);
		memcpy(frame_buffer->data+frame_buffer->size-4, TR_TAIL, 4);

		return frame_buffer;
	}
		
	case TRHEAD_DC:
	{
		dc_t *p_dc = (dc_t *)frame;
		frame_buffer = (tr_buffer_t *)calloc(1, sizeof(tr_buffer_t));
		frame_buffer->size = TR_DC_DATA_FIX_LEN+p_dc->data_len;
		if(frame_buffer->size > TR_BUFFER_SIZE)
		{
			free(frame_buffer);
			return NULL;
		}
		frame_buffer->data = (uint8 *)calloc(frame_buffer->size, sizeof(uint8));
		
		memcpy(frame_buffer->data, TR_HEAD_DC, 3);
		incode_xtocs(frame_buffer->data+3, p_dc->zidentify_no, 8);
		incode_xtocs(frame_buffer->data+19, p_dc->cidentify_no, 8);
		frame_buffer->data[35] = get_trtrans_to_ch(p_dc->trans_type);
		frame_buffer->data[36] = get_trinfo_to_ch(p_dc->tr_info);
		memcpy(frame_buffer->data+37, p_dc->data, p_dc->data_len);
		memcpy(frame_buffer->data+frame_buffer->size-4, TR_TAIL, 4);

		return frame_buffer;
	}
		
	case TRHEAD_UB:
	{
		ub_t *p_ub = (ub_t *)frame;
		frame_buffer = (tr_buffer_t *)calloc(1, sizeof(tr_buffer_t));
		frame_buffer->size = TR_UB_DATA_FIX_LEN+p_ub->data_len;
		if(frame_buffer->size > TR_BUFFER_SIZE)
		{
			free(frame_buffer);
			return NULL;
		}
		frame_buffer->data = (uint8 *)calloc(frame_buffer->size, sizeof(uint8));
		
		memcpy(frame_buffer->data, TR_HEAD_UB, 3);
		incode_xtocs(frame_buffer->data+3, p_ub->zidentify_no, 8);
		incode_xtocs(frame_buffer->data+19, p_ub->cidentify_no, 8);
		frame_buffer->data[35] = get_trtrans_to_ch(p_ub->trans_type);
		frame_buffer->data[36] = get_trinfo_to_ch(p_ub->tr_info);
		memcpy(frame_buffer->data+37, p_ub->data, p_ub->data_len);
		memcpy(frame_buffer->data+frame_buffer->size-4, TR_TAIL, 4);

		return frame_buffer;
	}
	
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
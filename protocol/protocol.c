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
#include <cJSON.h>
#include <module/netapi.h>
#include <protocol/common/fieldlysis.h>

/* Used for SuperButton Functions */
#define SB_OPT_CFG				'!'
#define SB_OPT_DEVREG			'@'
#define SB_OPT_PAIR				'#'
#define SB_OPT_PAIRREG			'$'
#define SB_OPT_MATCH			'%'
#define SB_OPT_RESET			'^'
#define SB_OPT_CTRL				'&'
#define SB_OPT_REMOTE_CTRL		'*'

#ifdef COMM_CLIENT
static int zdev_sync_zopt(dev_opt_t *dst_opt, uint8 *data, uint8 datalen);

void analysis_zdev_frame(void *ptr)
{
	frhandler_arg_t *arg = (frhandler_arg_t *)ptr;
	if(NULL == arg)
	{
		return;
	}

	fr_head_type_t head_type = get_frhead_from_str(arg->buf);
	void *p = get_frame_alloc(head_type, arg->buf, arg->len);
	if(NULL == p)
	{
		return;
	}

	switch(head_type)
	{
	case HEAD_UC:
	{
		uc_t *uc = (uc_t *)p;
		gw_info_t *p_gw = get_gateway_info();
		incode_ctoxs(p_gw->gw_no, uc->ext_addr, 16);
		get_gateway_info()->zapp_type = get_frapp_type_from_str(uc->ed_type);
		incode_ctox16(&(p_gw->zpanid), uc->panid);
		incode_ctox16(&(p_gw->zchannel), uc->channel);

		if(p_gw->zgw_opt != NULL)
		{
			free(p_gw->zgw_opt);
		}
		p_gw->zgw_opt = get_devopt_fromstr(p_gw->zapp_type, uc->data, uc->data_len);
		p_gw->rand = gen_rand(p_gw->gw_no);

		sync_gateway_info(p_gw);

		get_frame_free(HEAD_UC, uc);
	}
	break;
		
	case HEAD_UO:
	{
		uo_t *uo = (uo_t *)p;
		dev_info_t *dev_info = calloc(1, sizeof(dev_info_t));
		incode_ctoxs(dev_info->zidentity_no, uo->ext_addr, 16);
		incode_ctox16(&dev_info->znet_addr, uo->short_addr);
		dev_info->zapp_type = get_frapp_type_from_str(uo->ed_type);
		dev_info->znet_type = get_frnet_type_from_str(uo->type);

		dev_info->zdev_opt = 
			get_devopt_fromstr(dev_info->zapp_type, uo->data, uo->data_len);

		set_zdev_check(dev_info->znet_addr);
		uint16 znet_addr = dev_info->znet_addr;
		
		if(add_zdevice_info(dev_info) != 0)
		{
			get_zdev_frame_free(dev_info);
		}
		
		dev_info = query_zdevice_info(znet_addr);
		if(dev_info != NULL)
		{
			sync_zdev_info(dev_info);
		}
		
		get_frame_free(HEAD_UO, uo);
	}
	break;
		
	case HEAD_UH:
	{
		uh_t *uh = (uh_t *)p;
		uint16 znet_addr;
		incode_ctox16(&znet_addr, uh->short_addr);
		dev_info_t *dev_info = query_zdevice_info(znet_addr);
		if(dev_info == NULL)
		{
			uint8 mbuf[16] = {0};
			sprintf(mbuf, "D:/SR/%04X:O\r\n", znet_addr);
			serial_write(mbuf, 14);
		}
		else
		{
			set_zdev_check(znet_addr);
		}
		get_frame_free(HEAD_UH, uh);
	}
	break;
		
	case HEAD_UR:
	{
		ur_t *ur = (ur_t *)p;
		uint16 znet_addr;
		incode_ctox16(&znet_addr, ur->short_addr);
		if(znet_addr == 0)	//gateway
		{
			gw_info_t *p_gw = get_gateway_info();
			if(zdev_sync_zopt(p_gw->zgw_opt, ur->data, ur->data_len) == 0)
			{
				sync_gateway_info(p_gw);
			}
		}
		else	//device
		{
			dev_info_t *dev_info = query_zdevice_info(znet_addr);
			if(dev_info == NULL)
			{
				uint8 mbuf[16] = {0};
				sprintf(mbuf, "D:/SR/%04X:O\r\n", znet_addr);
				serial_write(mbuf, 14);
			}
			else
			{
				if(zdev_sync_zopt(dev_info->zdev_opt, ur->data, ur->data_len) ==0 )
				{
					sync_zdev_info(dev_info);
				}
			}
		}
		get_frame_free(HEAD_UR, ur);
	}
	break;
		
	case HEAD_DE:
	{
		de_t *de = (de_t *)p;
#ifdef BIND_SUPERBUTTON_CTRL_SUPPORT
		if(de->data_len > 17 && de->data[0] == SB_OPT_CFG)
		{
			int i = 0;
			int size = (de->data_len-17)/16;
			
			uint8 own_mac_str[18] = {0};
			memcpy(own_mac_str, de->data+1, 16);
				
			while(i < size)
			{
				int is_match = 0;
				zidentify_no_t cur_mac = {0};
				incode_ctoxs(cur_mac, de->data+17+16*i, 16);
				
				dev_info_t *p_dev = get_gateway_info()->p_dev;
				while(p_dev != NULL)
				{
					if(!memcmp(p_dev->zidentity_no, cur_mac, sizeof(zidentify_no_t)))
					{
						is_match = 1;
						break;
					}
					
					p_dev = p_dev->next;
				}

				if(is_match)
				{
					uint8 mbuf[32] = {0};
					sprintf(mbuf, "D:/FC/%04X%c%s:O\r\n", 
										p_dev->znet_addr,
										SB_OPT_DEVREG,
										own_mac_str);
					serial_write(mbuf, 31);
					usleep(200000);
				}
				else
				{}
				
				i++;
			}
		}
		else if(de->data_len > 36 && de->data[0] == SB_OPT_MATCH)
		{
			zidentify_no_t match_mac = {0};
			uint8 data_tail[24] = {0};
			
			incode_ctoxs(match_mac, de->data+1, 16);
			memcpy(data_tail, de->data+17, 20);

			dev_info_t *p_dev = get_gateway_info()->p_dev;
			while(p_dev != NULL)
			{
				if(!memcmp(p_dev->zidentity_no, match_mac, sizeof(zidentify_no_t)))
				{
					uint8 mbuf[56] = {0};	
					uint8 gw_mac[18] = {0};
					incode_xtocs(gw_mac, get_gateway_info()->gw_no, 8);
					sprintf(mbuf, "D:/FC/%04X%c%s%s:O\r\n", 
										p_dev->znet_addr, 
										SB_OPT_MATCH, 
										gw_mac, 
										data_tail);
					
					serial_write(mbuf, 51);
					break;
				}
				
				p_dev = p_dev->next;
			}
		}
#endif
		get_frame_free(HEAD_DE, de);
	}
	break;
		
	default: break;
	}

	get_frhandler_arg_free(arg);
}

int zdev_sync_zopt(dev_opt_t *dst_opt, uint8 *data, uint8 datalen)
{
	dev_opt_t *src_opt = get_devopt_fromstr(dst_opt->type, data, datalen);
	set_devopt_data_fromopt(dst_opt, src_opt);
	get_devopt_free(src_opt);
#ifdef DE_PRINT_SERIAL_PORT
	devopt_de_print(dst_opt);
#endif

	if((dst_opt->type == FRAPP_DOOR_SENSOR
		&& !dst_opt->device.doorsensor.setting)
		|| (dst_opt->type == FRAPP_IR_DETECTION
			&& !dst_opt->device.irdetect.setting)
		|| (dst_opt->type == FRAPP_ENVDETECTION
			&& !dst_opt->device.envdetection.up_setting)
		|| (dst_opt->type == FRAPP_AIRCONTROLLER
			&& !dst_opt->device.envdetection.up_setting))
	{
		return 1;
	}

	return 0;
}
#endif

void analysis_capps_frame(void *ptr)
{
	frhandler_arg_t *arg = (frhandler_arg_t *)ptr;
	if(arg == NULL)
	{
		return;
	}

	cJSON *pRoot = cJSON_Parse(arg->buf);
	if(pRoot == NULL)
	{
		goto capps_arg_end;
	}

	cJSON *pAction = cJSON_GetObjectItem(pRoot, JSON_FIELD_ACTION);
	if(pAction == NULL)
	{
		goto capps_cjson_end;
	}

	switch(atoi(pAction->valuestring))
	{
	case ACTION_TOCOLREQ:
	case ACTION_TOCOLRES:
	{
		cJSON *pProtocol = cJSON_GetObjectItem(pRoot, JSON_FIELD_PROTOCOL);
		if(pProtocol == NULL)
		{
			goto capps_cjson_end;
		}
		
		trfr_tocolreq_t *tocolreq = 
			get_trfr_tocolreq_alloc(atoi(pAction->valuestring),
										pProtocol->valuestring);

		trans_protocol_request_handler(arg, tocolreq);
		get_trfr_tocolreq_free(tocolreq);
	}
		break;

#ifdef COMM_SERVER
	case ACTION_REPORT:
	{
		cJSON *pGateway = cJSON_GetObjectItem(pRoot, JSON_FIELD_GWSN);
		if(pGateway == NULL)
		{
			goto capps_cjson_end;
		}

		cJSON *pDevs = cJSON_GetObjectItem(pRoot, JSON_FIELD_DEVICES);
		if(pDevs == NULL)
		{
			goto capps_cjson_end;
		}

		int i = 0;
		int dev_size = cJSON_GetArraySize(pDevs);
		trfield_device_t **devices = calloc(dev_size, sizeof(trfield_device_t *));
			
		while(i < dev_size)
		{
			cJSON *pDev = cJSON_GetArrayItem(pDevs, i);
			i++;
			if(pDev == NULL)
			{
				continue;
			}

			cJSON *pDevName = cJSON_GetObjectItem(pDev, JSON_FIELD_NAME);
			if(pDevName == NULL) continue;

			cJSON *pDevDevSN = cJSON_GetObjectItem(pDev, JSON_FIELD_DEVSN);
			if(pDevDevSN == NULL) continue;

			cJSON *pDevDevType = cJSON_GetObjectItem(pDev, JSON_FIELD_DEVTYPE);
			if(pDevDevType == NULL) continue;

			cJSON *pDevZnetStatus = cJSON_GetObjectItem(pDev, JSON_FIELD_ZSTATUS);
			if(pDevZnetStatus == NULL) continue;

			cJSON *pDevData = cJSON_GetObjectItem(pDev, JSON_FIELD_DEVDATA);
			if(pDevData == NULL) continue;

			*(devices + i - 1) = 
				get_trfield_device_alloc(pDevName->valuestring, 
											pDevDevSN->valuestring, 
											pDevDevType->valuestring, 
											pDevZnetStatus->valuestring, 
											pDevData->valuestring);
		}

		trfr_report_t *report = 
			get_trfr_report_alloc(atoi(pAction->valuestring),
									pGateway->valuestring, 
									devices, 
									dev_size);

		trans_report_handler(arg, report);
		get_trfr_report_free(report);
	}
		break;

	case ACTION_CHECK:
	{
		cJSON *pGateway = cJSON_GetObjectItem(pRoot, JSON_FIELD_GWSN);
		if(pGateway == NULL)
		{
			goto capps_cjson_end;
		}

		cJSON *pCodeCheck = cJSON_GetObjectItem(pRoot, JSON_FIELD_CODECHECK);
		if(pCodeCheck == NULL)
		{
			goto capps_cjson_end;
		}

		cJSON *pCodeData = cJSON_GetObjectItem(pRoot, JSON_FIELD_CODEDATA);
		if(pCodeData == NULL)
		{
			goto capps_cjson_end;
		}

		cJSON *pDevSNs = cJSON_GetObjectItem(pRoot, JSON_FIELD_DEVSNS);
		if(pDevSNs == NULL)
		{
			goto capps_cjson_end;
		}

		int i = 0;
		int devsn_size = cJSON_GetArraySize(pDevSNs);
		sn_t *dev_sns = calloc(devsn_size, sizeof(sn_t));

		while(i < devsn_size)
		{
			cJSON *pDevSN = cJSON_GetArrayItem(pDevSNs, i);
			i++;
			if(pDevSN == NULL) continue;

			STRS_MEMCPY(dev_sns+i-1, pDevSN->valuestring, 
					sizeof(sn_t), strlen(pDevSN->valuestring));
		}

		trfr_check_t *check = 
			get_trfr_check_alloc(atoi(pAction->valuestring), 
									pGateway->valuestring, 
									dev_sns, 
									devsn_size, 
									pCodeCheck->valuestring, 
									pCodeData->valuestring);

		trans_check_handler(arg, check);
		get_trfr_check_free(check);
	}
		break;

	case ACTION_RESPOND:
	{
		cJSON *pGateway = cJSON_GetObjectItem(pRoot, JSON_FIELD_GWSN);
		if(pGateway == NULL)
		{
			goto capps_cjson_end;
		}

		cJSON *pDevSN = cJSON_GetObjectItem(pRoot, JSON_FIELD_DEVSN);
		if(pDevSN == NULL)
		{
			goto capps_cjson_end;
		}

		cJSON *pDevData = cJSON_GetObjectItem(pRoot, JSON_FIELD_DEVDATA);
		if(pDevData == NULL)
		{
			goto capps_cjson_end;
		}

		trfr_respond_t *respond = 
			get_trfr_respond_alloc(atoi(pAction->valuestring),
										pGateway->valuestring, 
										pDevSN->valuestring, 
										pDevData->valuestring);

		trans_respond_handler(arg, respond);
		get_trfr_respond_free(respond);
	}
		break;
#endif
#ifdef COMM_CLIENT
	case ACTION_REFRESH:
	{
		cJSON *pGateway = cJSON_GetObjectItem(pRoot, JSON_FIELD_GWSN);
		if(pGateway == NULL)
		{
			goto capps_cjson_end;
		}

		cJSON *pDevSNs = cJSON_GetObjectItem(pRoot, JSON_FIELD_DEVSNS);
		if(pDevSNs == NULL)
		{
			goto capps_cjson_end;
		}

		int i = 0;
		int devsn_size = cJSON_GetArraySize(pDevSNs);
		sn_t *dev_sns = calloc(devsn_size, sizeof(sn_t));

		while(i < devsn_size)
		{
			cJSON *pDevSN = cJSON_GetArrayItem(pDevSNs, i);
			i++;
			if(pDevSN == NULL) continue;

			STRS_MEMCPY(dev_sns+i-1, pDevSN->valuestring, 
					sizeof(sn_t), strlen(pDevSN->valuestring));
		}

		trfr_refresh_t *refresh = 
			get_trfr_refresh_alloc(atoi(pAction->valuestring),
									pGateway->valuestring, 
									dev_sns, 
									devsn_size);

		trans_refresh_handler(arg, refresh);
		get_trfr_refresh_free(refresh);
	}
		break;

	case ACTION_CONTROL:
	{
		cJSON *pGateway = cJSON_GetObjectItem(pRoot, JSON_FIELD_GWSN);
		if(pGateway == NULL)
		{
			goto capps_cjson_end;
		}

		cJSON *pDevSNs = cJSON_GetObjectItem(pRoot, JSON_FIELD_DEVSNS);
		if(pDevSNs == NULL)
		{
			goto capps_cjson_end;
		}

		cJSON *pCmd = cJSON_GetObjectItem(pRoot, JSON_FIELD_CMD);
		if(pCmd == NULL)
		{
			goto capps_cjson_end;
		}

		int i = 0;
		int devsn_size = cJSON_GetArraySize(pDevSNs);
		sn_t *dev_sns = calloc(devsn_size, sizeof(sn_t));

		while(i < devsn_size)
		{
			cJSON *pDevSN = cJSON_GetArrayItem(pDevSNs, i);
			i++;
			if(pDevSN == NULL) continue;

			STRS_MEMCPY(dev_sns+i-1, pDevSN->valuestring, 
					sizeof(sn_t), strlen(pDevSN->valuestring));
		}

		trfr_control_t *control = 
			get_trfr_control_alloc(atoi(pAction->valuestring), 
									pGateway->valuestring, 
									dev_sns, 
									devsn_size,
									pCmd->valuestring);

		trans_control_handler(arg, control);
		get_trfr_control_free(control);
	}
		break;
#endif
	}

capps_cjson_end:
	cJSON_Delete(pRoot);
capps_arg_end:
	get_frhandler_arg_free(arg);
}


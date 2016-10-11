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
#include <module/serial.h>
#include <protocol/request.h>
#include <protocol/common/mevent.h>
#include <protocol/common/fieldlysis.h>

#ifdef __cplusplus
extern "C" {
#endif

void analysis_zdev_frame(void *ptr)
{
	frhandler_arg_t *arg = (frhandler_arg_t *)ptr;
	if(NULL == arg)
	{
		return;
	}

	//DE_PRINTF(0, "%s\nserial read: ", get_time_head());
	//PRINT_HEX(arg->buf, arg->len);

	zidentify_no_t devno;
	memcpy(devno, get_gateway_info()->gw_no+1, sizeof(zidentify_no_t)-1);
	devno[sizeof(zidentify_no_t)-1] = arg->buf[0];

	dev_info_t *dev_info = (dev_info_t *)calloc(1, sizeof(dev_info_t));
	memcpy(dev_info->dev_no, devno, sizeof(zidentify_no_t));
	dev_info->type = FRAPP_CONNECTOR;
	memcpy(dev_info->data, arg->buf+3, arg->buf[2]);
	dev_info->ischange = 1;

	if(add_zdevice_info(dev_info) != 0)
	{
		free(dev_info);
	}

	upload_data(NULL);

	get_frhandler_arg_free(arg);
}

void analysis_capps_frame(void *ptr)
{
	cJSON *pAction = NULL;
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

	pAction = cJSON_GetObjectItem(pRoot, JSON_FIELD_ACTION);
	if(pAction == NULL)
	{
		goto capps_cjson_end;
	}

	switch(atoi(pAction->valuestring))
	{
#if defined(COMM_TARGET)
	case ACTION_REFRESH:
	{
		trfield_obj_t *obj = NULL;
		cJSON *pObj = cJSON_GetObjectItem(pRoot, JSON_FIELD_OBJECT);
		if(pObj != NULL)
		{
			cJSON *pOwner = cJSON_GetObjectItem(pRoot, JSON_FIELD_OWNER);
			cJSON *pCustom = cJSON_GetObjectItem(pRoot, JSON_FIELD_CUSTOM);
			if(pOwner != NULL && pCustom != NULL)
			{
				obj = get_trfield_obj_alloc(pOwner->valuestring, pCustom->valuestring);
			}
		}

		cJSON *pGateway = cJSON_GetObjectItem(pRoot, JSON_FIELD_GWSN);
		if(pGateway == NULL)
		{
			goto capps_cjson_end;
		}

		cJSON *pRandom = cJSON_GetObjectItem(pRoot, JSON_FIELD_RANDOM);
		if(pRandom == NULL)
		{
			goto capps_cjson_end;
		}

		int devsn_size = 0;
		sn_t *dev_sns = NULL;
		cJSON *pDevSNs = cJSON_GetObjectItem(pRoot, JSON_FIELD_DEVSNS);
		if(pDevSNs != NULL)
		{
			int i = 0;
			devsn_size = cJSON_GetArraySize(pDevSNs);
			dev_sns = (sn_t *)calloc(devsn_size, sizeof(sn_t));

			while(i < devsn_size)
			{
				cJSON *pDevSN = cJSON_GetArrayItem(pDevSNs, i);
				i++;
				if(pDevSN == NULL) continue;

				STRS_MEMCPY(dev_sns+i-1, pDevSN->valuestring, 
						sizeof(sn_t), strlen(pDevSN->valuestring));
			}
		}

		trfr_refresh_t *refresh = 
			get_trfr_refresh_alloc(obj,
									pGateway->valuestring, 
									dev_sns, 
									devsn_size,
									pRandom->valuestring);

		trans_refresh_handler(refresh);
		get_trfr_refresh_free(refresh);
	}
		break;

	case ACTION_CONTROL:
	{
		trfield_obj_t *obj = NULL;
		cJSON *pObj = cJSON_GetObjectItem(pRoot, JSON_FIELD_OBJECT);
		if(pObj != NULL)
		{
			cJSON *pOwner = cJSON_GetObjectItem(pRoot, JSON_FIELD_OWNER);
			cJSON *pCustom = cJSON_GetObjectItem(pRoot, JSON_FIELD_CUSTOM);
			if(pOwner != NULL && pCustom != NULL)
			{
				obj = get_trfield_obj_alloc(pOwner->valuestring, pCustom->valuestring);
			}
		}

		char *pgw_str = NULL;
		cJSON *pGateway = cJSON_GetObjectItem(pRoot, JSON_FIELD_GWSN);
		if(pGateway != NULL)
		{
			pgw_str = pGateway->valuestring;
		}

		cJSON *pRandom = cJSON_GetObjectItem(pRoot, JSON_FIELD_RANDOM);
		if(pRandom == NULL)
		{
			goto capps_cjson_end;
		}

		cJSON *pCtrls = cJSON_GetObjectItem(pRoot, JSON_FIELD_CTRLS);
		if(pCtrls == NULL)
		{
			goto capps_cjson_end;
		}

		int ctrl_size;
		trfield_ctrl_t **ctrls;
		if(pCtrls == NULL)
		{
			ctrl_size = 0;
			ctrls = NULL;
		}
		else
		{
			ctrl_size = cJSON_GetArraySize(pCtrls);
			ctrls = (trfield_ctrl_t **)calloc(ctrl_size, sizeof(trfield_ctrl_t *));
		}

		int i = 0;			
		while(i < ctrl_size)
		{
			cJSON *pCtrl = cJSON_GetArrayItem(pCtrls, i);
			i++;
 			if(pCtrl == NULL) continue;

			cJSON *pCmd = cJSON_GetObjectItem(pCtrl, JSON_FIELD_CMD);
			if(pCmd == NULL) continue;

			cJSON *pDevSN = cJSON_GetObjectItem(pCtrl, JSON_FIELD_DEVSN);
			if(pDevSN == NULL)
			{
				continue;
			}

			*(ctrls + i - 1) = 
				get_trfield_ctrl_alloc(pDevSN->valuestring, pCmd->valuestring);
		}

		trfr_control_t *control = 
			get_trfr_control_alloc(obj,
									pgw_str,
									ctrls,
									ctrl_size,
									pRandom->valuestring);

		trans_control_handler(control);
		get_trfr_control_free(control);
	}
		break;
#endif
	case ACTION_TOCOLRES:
	{
		trfield_obj_t *obj = NULL;
		cJSON *pObj = cJSON_GetObjectItem(pRoot, JSON_FIELD_OBJECT);
		if(pObj != NULL)
		{
			cJSON *pOwner = cJSON_GetObjectItem(pRoot, JSON_FIELD_OWNER);
			cJSON *pCustom = cJSON_GetObjectItem(pRoot, JSON_FIELD_CUSTOM);
			if(pOwner != NULL && pCustom != NULL)
			{
				obj = get_trfield_obj_alloc(pOwner->valuestring, pCustom->valuestring);
			}
		}

		cJSON *pReqAction = cJSON_GetObjectItem(pRoot, JSON_FIELD_REQACTION);
		if(pReqAction == NULL)
		{
			goto capps_cjson_end;
		}

		char *info_str = NULL;
		cJSON *pInfo = cJSON_GetObjectItem(pRoot, JSON_FIELD_INFO);
		if(pInfo != NULL)
		{
			info_str = pInfo->valuestring;
		}

		cJSON *pRandom = cJSON_GetObjectItem(pRoot, JSON_FIELD_RANDOM);
		if(pRandom == NULL)
		{
			goto capps_cjson_end;
		}

		trfr_tocolres_t *tocolres =
			get_trfr_tocolres_alloc(obj,
										(trans_action_t)atoi(pReqAction->valuestring),
										info_str,
										pRandom->valuestring);
		trans_tocolres_handler(tocolres);
		get_trfr_tocolres_free(tocolres);
	}
		break;
	}

capps_cjson_end:
	cJSON_Delete(pRoot);
capps_arg_end:
	get_frhandler_arg_free(arg);
}

#ifdef __cplusplus
}
#endif


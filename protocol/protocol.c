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
void analysis_zdev_frame(void *ptr)
{
	frhandler_arg_t *arg = (frhandler_arg_t *)ptr;
	if(arg == NULL)
	{
		return;
	}
	
	dev_info_t *dev_info;
	uint16 znet_addr;

	char ipaddr[24] = {0};
	GET_UDP_SERVICE_IPADDR(ipaddr);
	
	fr_head_type_t head_type = get_frhead_from_str(arg->buf);
	
	void *p = get_frame_alloc(head_type, arg->buf, arg->len);

	if(p == NULL)
	{
		return;
	}

	switch(head_type)
	{
	case HEAD_UC:
	{
		uc_t *uc = (uc_t *)p;
		incode_ctoxs(get_gateway_info()->gw_no, uc->ext_addr, 16);
		get_gateway_info()->zapp_type = get_frapp_type_from_str(uc->ed_type);
		incode_ctox16(&(get_gateway_info()->zpanid), uc->panid);
		incode_ctox16(&(get_gateway_info()->zchannel), uc->channel);
		get_gateway_info()->zgw_opt = 
			get_devopt_data_alloc(get_gateway_info()->zapp_type, 
													uc->data, uc->data_len);
		get_gateway_info()->rand = gen_rand(get_gateway_info()->gw_no);
		
		fr_buffer_t *buffer = get_gateway_buffer_alloc(get_gateway_info());
		
		get_buffer_free(buffer);
		get_frame_free(HEAD_UC, uc);
	}
	break;
		
	case HEAD_UO:
	{
		uo_t *uo = (uo_t *)p;
		dev_info = calloc(1, sizeof(dev_info_t));
		incode_ctoxs(dev_info->zidentity_no, uo->ext_addr, 16);
		incode_ctox16(&dev_info->znet_addr, uo->short_addr);
		dev_info->zapp_type = get_frapp_type_from_str(uo->ed_type);
		dev_info->znet_type = get_frnet_type_from_str(uo->type);

		dev_info->zdev_opt = get_devopt_data_alloc(dev_info->zapp_type, 
													uo->data, uo->data_len);
		
#ifdef DE_ZDEVICE_RECORD
		FILE *fp = NULL;
		if((fp = fopen(RECORD_FILE, "a+")) != NULL)
		{
			char sn[20] = {0};
			incode_xtocs(sn, dev_info->zidentity_no, 8);
			fprintf(fp, "[zdevice]\nS/N: %s\nShort Addr: %04X\nDev Type: %02d\n\n", 
				sn, dev_info->znet_addr, dev_info->zapp_type);
			fclose(fp);
		}
#endif
		set_zdev_check(dev_info->znet_addr);
		uint16 znet_addr = dev_info->znet_addr;
		
		if(add_zdevice_info(dev_info) != 0)
		{
			get_devopt_data_free(dev_info->zdev_opt);
			free(dev_info);
		}
		else
		{
			uint8 *buffer = get_zdev_buffer_alloc(dev_info);
			get_zdev_buffer_free(buffer);
		}

		dev_info = query_zdevice_info(znet_addr);
		if(dev_info != NULL)
		{
			fr_buffer_t *frbuffer = get_switch_buffer_alloc(HEAD_UO, 
											dev_info->zdev_opt, uo);
		
			cli_info_t *p_cli = get_client_list()->p_cli;
			while(p_cli != NULL)
			{
				p_cli = p_cli->next;
			}
			get_buffer_free(frbuffer);
		}
		
		get_frame_free(HEAD_UO, uo);
	}
	break;
		
	case HEAD_UH:
	{
		uh_t *uh = (uh_t *)p;
		incode_ctox16(&znet_addr, uh->short_addr);
		dev_info = query_zdevice_info(znet_addr);
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
		fr_buffer_t *frbuffer = NULL;
		ur_t *ur = (ur_t *)p;
		incode_ctox16(&znet_addr, ur->short_addr);
		if(znet_addr == 0)
		{
			dev_opt_t *opt = get_devopt_data_alloc(get_gateway_info()->zapp_type, 
														ur->data, ur->data_len);
			set_devopt_data_fromopt(get_gateway_info()->zgw_opt, opt);
			get_devopt_data_free(opt);
#ifdef DE_PRINT_SERIAL_PORT
			devopt_de_print(get_gateway_info()->zgw_opt);
#endif
			if((get_gateway_info()->zgw_opt->type == FRAPP_DOOR_SENSOR
				&& !get_gateway_info()->zgw_opt->device.doorsensor.setting)
				|| (get_gateway_info()->zgw_opt->type == FRAPP_IR_DETECTION
					&& !get_gateway_info()->zgw_opt->device.irdetect.setting)
				|| (get_gateway_info()->zgw_opt->type == FRAPP_ENVDETECTION
					&& !get_gateway_info()->zgw_opt->device.envdetection.up_setting)
				|| (get_gateway_info()->zgw_opt->type == FRAPP_AIRCONTROLLER
					&& !get_gateway_info()->zgw_opt->device.envdetection.up_setting))
			{
				goto UR_FREE;
			}
			
			frbuffer = get_switch_buffer_alloc(HEAD_UR, 
								get_gateway_info()->zgw_opt, ur);
				
		}
		else
		{
			dev_info = query_zdevice_info(znet_addr);
			if(dev_info == NULL)
			{
				uint8 mbuf[16] = {0};
				sprintf(mbuf, "D:/SR/%04X:O\r\n", znet_addr);
				serial_write(mbuf, 14);
			}
			else
			{
				dev_opt_t *opt = get_devopt_data_alloc(dev_info->zapp_type, 
														ur->data, ur->data_len);
				set_devopt_data_fromopt(dev_info->zdev_opt, opt);
				get_devopt_data_free(opt);
#ifdef DE_PRINT_SERIAL_PORT
				devopt_de_print(dev_info->zdev_opt);
#endif
				if((dev_info->zdev_opt->type == FRAPP_DOOR_SENSOR
					&& !dev_info->zdev_opt->device.doorsensor.setting)
					|| (dev_info->zdev_opt->type == FRAPP_IR_DETECTION
						&& !dev_info->zdev_opt->device.irdetect.setting)
					|| (dev_info->zdev_opt->type == FRAPP_ENVDETECTION
						&& !dev_info->zdev_opt->device.envdetection.up_setting)
					|| (dev_info->zdev_opt->type == FRAPP_AIRCONTROLLER
						&& !dev_info->zdev_opt->device.envdetection.up_setting))
				{
					goto UR_FREE;
				}

				frbuffer = get_switch_buffer_alloc(HEAD_UR, 
												dev_info->zdev_opt, ur);
			}
		}

		if(frbuffer != NULL)
		{
			cli_info_t *p_cli = get_client_list()->p_cli;
			while(p_cli != NULL)
			{
				p_cli = p_cli->next;
			}
			get_buffer_free(frbuffer);
		}
UR_FREE:
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
#endif

void analysis_capps_frame(void *ptr)
{
	frhandler_arg_t *arg = (frhandler_arg_t *)ptr;
	if(arg == NULL)
	{
		return;
	}
	
	get_frhandler_arg_free(arg);
}


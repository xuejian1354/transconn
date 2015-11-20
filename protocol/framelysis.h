/*
 * framelysis.h
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
#ifndef __FRAMELYSIS_H__
#define __FRAMELYSIS_H__

#include <services/globals.h>

#define ZH_TYPE_NAME

//frame head
#define FR_HEAD_UC	"UC:"	//coord up data
#define FR_HEAD_UO	"UO:"	//device up data
#define FR_HEAD_UH	"UH:"	//device heart beat
#define FR_HEAD_DE	"D:"	//command
#define FR_HEAD_UR	"UR:"	//command return
#define FR_HEAD_UJ	"UJ:"	//permit join


//net device type
#define FR_DEV_COORD	'0'
#define FR_DEV_ROUTER	'0'
#define FR_DEV_ENDDEV	'1'

//app device type
#define FR_APP_CONNECTOR			"00"
#define FR_APP_ENDNODE				"00"
#define FR_APP_LIGHTSWITCH_ONE		"01"
#define FR_APP_LIGHTSWITCH_TWO		"02"
#define FR_APP_LIGHTSWITCH_THREE	"03"
#define FR_APP_LIGHTSWITCH_FOUR		"04"
#define FR_APP_HUELIGHT				"05"
#define FR_APP_ALARM				"11"
#define FR_APP_IR_DETECTION			"12"
#define FR_APP_DOOR_SENSOR			"13"
#define FR_APP_ENVDETECTION			"14"
#define FR_APP_IR_RELAY				"21"
#define FR_APP_AIRCONTROLLER		"F0"
#define FR_APP_RELAYSOCKET			"F1"
#define FR_APP_HUMITURE_DETECTION	"A1"
#define FR_APP_SOLENOID_VALVE		"A2"

//type for smartlab device
#define FR_APP_LAMPSWITCH		"31"
#define FR_APP_PROJECTOR		"32"
#define FR_APP_AIRCONDITION		"33"
#define FR_APP_CURTAIN			"34"
#define FR_APP_DOORLOCK			"35"

#define FR_TAIL ":O\r\n"

typedef enum
{
	HEAD_UC,
	HEAD_UO,
	HEAD_UH,
	HEAD_UR,
	HEAD_DE,
	HEAD_NONE,
}fr_head_type_t;

typedef enum
{
	FRAPP_CONNECTOR = 0,
	FRAPP_LIGHTSWITCH_ONE = 1,
	FRAPP_LIGHTSWITCH_TWO = 2,
	FRAPP_LIGHTSWITCH_THREE = 3,
	FRAPP_LIGHTSWITCH_FOUR = 4,
	FRAPP_HUELIGHT = 5,
	FRAPP_ALARM = 11,
	FRAPP_IR_DETECTION = 12,
	FRAPP_DOOR_SENSOR = 13,
	FRAPP_ENVDETECTION = 14,
	FRAPP_IR_RELAY = 21,
	FRAPP_AIRCONTROLLER = 0xF0,
	FRAPP_RELAYSOCKET = 0xF1,
	FRAPP_HUMITURE_DETECTION = 0xA1,
	FRAPP_SOLENOID_VALVE = 0xA2,

	//type for smartlab device
	FRAPP_LAMPSWITCH = 31,
	FRAPP_PROJECTOR = 32,
	FRAPP_AIRCONDITION = 33,
	FRAPP_CURTAIN = 34,
	FRAPP_DOORLOCK = 35,
	
	FRAPP_NONE = 0xFF,
}fr_app_type_t;

typedef enum
{
    TN_GW,
    TN_SW1,
    TN_SW2,
    TN_SW3,
    TN_SW4,
    TN_HUE,
    TN_ALARM,
    TN_IRDECT,
    TN_DOORS,
    TN_PM25,
    TN_IRRELAY,
    TN_AIRCON,
    TN_RELAY,
    TN_HT,
    TN_VALVE,
    TN_LAMPSW,
	TN_PJECTOR,
	TN_AIRCONDITION,
	TN_CURTAIN,
	TN_DOORLOCK,
    TN_NUMS
}type_name_t;


typedef enum
{
	FRNET_ROUTER = 0,
	FRNET_ENDDEV = 1,
	FRNET_NONE,
}fr_net_type_t;

typedef struct
{
    fr_app_type_t type;
    char *val;
}zh_el_t;


//Coordinator info frame
typedef struct
{
	uint8 head[3];   //UC:
	uint8 type;   //net type, 0 support route
	uint8 ed_type[2]; //app type, 00 coordinator
	uint8 short_addr[4];   //net address
	uint8 ext_addr[16];  //mac address
	uint8 panid[4];    //net panid
	uint8 channel[4]; //net channel
	uint8 data_len;
	uint8 *data; //data
	uint8 tail[4];    //:O/r/n
}uc_t;


//Device info frame
typedef struct
{
	uint8 head[3];   //UO:
	uint8 type;   //net type
	uint8 ed_type[2]; //app type
	uint8 short_addr[4];   //net address
	uint8 ext_addr[16];  //mac address
	uint8 data_len;
	uint8 *data; //data
	uint8 tail[4];    //:O/r/n
}uo_t; 

 
 //Device Heart Beat frame
typedef struct
{
	uint8 head[3];   //UH:
	uint8 short_addr[4];   //net address
	uint8 tail[4];    //:O/r/n
}uh_t;  
 
 
//Device returns frame
typedef struct
{
	uint8 head[3];   //UR:
	uint8 type;   //net type
	uint8 ed_type[2]; //app type
	uint8 short_addr[4];   //net address
	uint8 data_len;
	uint8 *data;    //data
	uint8 tail[4];    //:O/r/n
}ur_t;  
 
 
//Device control frame
typedef struct
{
	uint8 head[2];   //D:
	uint8 cmd[4];   //cmmand
	uint8 short_addr[4];   //net address
	uint8 data_len;
	uint8 *data; //data
	uint8 tail[4]; //:O/r/n
 }de_t; 


typedef struct
{
	uint8 *data;
	uint8 size;
}fr_buffer_t;

char *get_name(type_name_t tn);
char *get_name_from_type(fr_app_type_t type);
char *get_mix_name(fr_app_type_t type, uint8 s1, uint8 s2);

fr_head_type_t get_frhead_from_str(char *head);
int get_frhead_to_str(char *dst, fr_head_type_t head_type);

fr_app_type_t get_frapp_type_from_str(char *app_type);
int get_frapp_type_to_str(char *dst, fr_app_type_t app_type);

fr_net_type_t get_frnet_type_from_str(char net_type);
char get_frnet_type_to_ch(fr_net_type_t net_type);


void *get_frame_alloc(fr_head_type_t htype, uint8 buffer[], int length);
void get_frame_free(fr_head_type_t htype, void *p);

fr_buffer_t *get_buffer_alloc(fr_head_type_t htype, void *frame);
void get_buffer_free(fr_buffer_t *p);

#endif	//  __FRAMELYSIS_H__

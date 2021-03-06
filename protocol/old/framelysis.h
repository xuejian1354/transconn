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
#include <protocol/common/fieldlysis.h>

#ifdef __cplusplus
extern "C" {
#endif

//command method
#define FR_CMD_BROCAST_REFRESH	"/BR/"		//broadcast
#define FR_CMD_SINGLE_REFRESH	"/SR/"		//single refresh
#define FR_CMD_SINGLE_EXCUTE	"/EC/"		//single control
#define FR_CMD_PEROID_EXCUTE	"/EP/"		//recycle request
#define FR_CMD_PEROID_STOP		"/ES/"		//recycle stop request
#define FR_CMD_JOIN_CTRL		"/CJ/"		//join permit

#define FR_UC_DATA_FIX_LEN		38		//UC frame fix len
#define FR_UO_DATA_FIX_LEN		30		//UO frame fix len
#define FR_UH_DATA_FIX_LEN		11		//UH frame fix len
#define FR_UR_DATA_FIX_LEN		14		//HR frame fix len
#define FR_DE_DATA_FIX_LEN		14		//DE frame fix len


#define FRAME_DATA_SIZE		128
#define FRAME_BUFFER_SIZE 	256
#define MAX_OPTDATA_SIZE	FRAME_DATA_SIZE


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
	FRNET_ROUTER = 0,
	FRNET_ENDDEV = 1,
	FRNET_NONE,
}fr_net_type_t;


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

fr_net_type_t get_frnet_type_from_str(char net_type);
char get_frnet_type_to_ch(fr_net_type_t net_type);


void *get_frame_alloc(fr_head_type_t htype, uint8 buffer[], int length);
void get_frame_free(fr_head_type_t htype, void *p);

fr_buffer_t *get_buffer_alloc(fr_head_type_t htype, void *frame);
void get_buffer_free(fr_buffer_t *p);

#ifdef __cplusplus
}
#endif

#endif	//  __FRAMELYSIS_H__

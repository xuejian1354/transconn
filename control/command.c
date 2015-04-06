/*
 * command.c
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
#include "command.h"
#include <pthread.h>

#ifdef DEBUG_CONSOLE_SUPPORT
void *console_control(void *p);
int split_ip_target(char *dst, char *src, int len);
void show_data(int type, char *data, int len);
int strtox(unsigned char *dst, char *pst, int size);

void create_console()
{
	pthread_t pthr;
	pthread_create(&pthr, NULL, console_control, NULL);
	DE_PRINTF("program into console control, you can input command to do something!\n");
}

void *console_control(void *p)
{
	char inputbuf[MAXSIZE];
	char cmd[8];
	unsigned char data[MAXSIZE];
	int i, usrlen, datalen;
	int temptype, dtype = 1;		// 1 string , 0 hex

	char ipaddr[24] = {0};
	sprintf(ipaddr, "%s:%d", SERVER_IP, TRANS_UDP_PORT);
	
	while(1)
	{
		memset(inputbuf, 0, sizeof(inputbuf));
		fgets(inputbuf, sizeof(inputbuf), stdin);

		usrlen = strlen(inputbuf);
		i = 0;
		memset(cmd, 0, sizeof(cmd));
		memset(data, 0, sizeof(data));
		
		while(i<usrlen && inputbuf[i++]!=' ')
		{
			cmd[i-1] = inputbuf[i-1];
		}
		
		while(i<usrlen && inputbuf[i++]==' ');

		if(i < usrlen)
		{
			if(!strncmp("type", cmd, 4) && strlen(cmd)==4)
			{
				temptype = dtype;
				dtype = 1;	
			}
			
			if(dtype == 1)		//string
			{
				strncpy(data, inputbuf+i-1, usrlen-i);
				datalen =strlen(data);
			}
			else if(dtype == 0)	//hex
			{
				datalen = strtox(data, inputbuf+i-1, usrlen-i);
			}
		}

		if(!strncmp("\n", cmd, 1))
		{
			goto	nextconsoleio;
		}
		else if(!strncmp("clear", cmd, 5) && strlen(cmd)==6)
		{
			system("clear");
			goto	nextconsoleio;
		}

		printf("\n----------------------------------------------------------------\n");
#ifdef SERIAL_SUPPORT		
		if(!strncmp("com", cmd, 3) && strlen(cmd)==3)
		{
			serial_write(data, datalen);
			printf("send to serial ");
			show_data(dtype, data, datalen);
		}
		else 
#endif
#ifdef TRANS_UDP_SERVICE
		if(!strncmp("udp", cmd, 3) && strlen(cmd)==3)
		{
			int dataloc = split_ip_target(ipaddr, data, datalen);
			socket_udp_sendto(ipaddr, data+dataloc, datalen-dataloc);
		}
		else 
#endif
#ifdef TRANS_TCP_SERVER
		if(!strncmp("tcp", cmd, 3) && strlen(cmd)==3)
		{

		}
		else 
#endif
		if(!strncmp("type", cmd, 4) && strlen(cmd)==4)
		{
			if(!strncmp(data, "hex", 3) && strlen(data)==3)
			{
				dtype = 0;
				printf("set data type hex\n\n");
			}
			else if(!strncmp(data, "str", 3) && strlen(data)==3)
			{
				dtype = 1;
				printf("set data type str\n\n");
			}
			else
			{
				dtype = temptype;
				printf("set type error,the correct type should be \"str\" or \"hex\"\n\n");
			}
		}
		else
		{
			printf("format error, input help to show details!\n\n");
		}
nextconsoleio:;
	}
}

int split_ip_target(char *dst, char *src, int len)
{
    int i, j;
    for(i=0; i<len; i++)
    {
        if('t' == src[i] && !strncmp(src+i, "target=", 7))
        {
            i += 7;
            for(j=i; j<len; j++)
            {
                if(' ' == src[j])
                {
                	break;
                }
            }
            strncpy(dst, src+i, j-i);
			*(dst+j) = '\0';
            return j+1;
        }
    }

	return 0;
}

void show_data(int type, char *data, int len)
{
    if(type == 1)
    {
        printf("str data, size=%d\n%s\n", len, data);
    }
    else if(type == 0)
    {
        int x;
        printf("hex data, size=%d\n", len);
        for(x=0; x<len; x++)
        {
            if(data[x] < 0x10)
                printf("0%x ", data[x]);
            else
                printf("%x ", data[x]);
        }
        printf("\n");
    }
    printf("\n");
}


//char to hex
int strtox(unsigned char *dst, char *pst, int size)
{
	unsigned char s = 0;
	int i, x=0;
	int m_size = size;
	int hasNum = 0;
	char *pstr = pst;

	while(m_size--)
	{
	    if((*pstr>='0')&&(*pstr<='9'))
		{
			i = *pstr-'0';
		}
		else if((*pstr>='a')&&(*pstr<='f'))
		{
			i = *pstr - 'a' + 0xa;
		}
		else if(*pstr>='A'&&*pstr<='F')
		{
			i = *pstr - 'A' + 0xa;
		}
		else
		{			
			if(hasNum == 1)
			{	
				dst[x++] = s;
				hasNum = 0;		
				s = 0;
			}
			goto strnext;
		}
		hasNum = 1;
		s = s*0x10 + i;
strnext:	pstr++;
	}
	
	if(hasNum == 1)
	{
		dst[x++] = s;
		hasNum = 0;
		s = 0;
	}
	
	return x;
}
#endif

/*
 * client.c
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
#include <services/globals.h>
#include <protocol/trframelysis.h>
#include <pthread.h>

void *client_control(void *p);
int strtox(unsigned char *dst, char *pst, int size);

static char target_ipaddr[24] = {0};

int main(int argc, char **argv)
{
	pthread_t pthr;
	int udp_port = TRANS_CLIENT_TEST_UDP_PORT;

	if(argc > 1)
		udp_port = atoi(argv[1]);

	if(argc > 2)
	{
		DE_PRINTF("Usage: %s [UDP Port]\n", TARGET_NAME);
		return -1;
	}
	
	DE_PRINTF("%s start!\n", TARGET_NAME);
	DE_PRINTF("UDP Port:%d\n", udp_port);	

#ifdef THREAD_POOL_SUPPORT
	if (tpool_create(TRANS_THREAD_MAX_NUM) < 0)
	{
		return -1;
	}
#endif

#ifdef TIMER_SUPPORT
	if(timer_init() < 0)
	{
		return -1;
	}
#endif

#ifdef SELECT_SUPPORT
	select_init();
#endif

#ifdef TRANS_UDP_SERVICE
	if (socket_udp_service_init(udp_port) < 0)
	{
		return -1;
	}
#endif

	if(mach_init() < 0)
	{
		return -1;
	}
	
	event_init();

	pthread_create(&pthr, NULL, client_control, NULL);
	DE_PRINTF("program into client control, you can input command to do something!\n");

	while(1)
	{
#ifdef SELECT_SUPPORT
		select_listen();
#endif
	}

end:	
	printf("end!(#>_<#)\n");
	return 0;
}

void *client_control(void *p)
{
	char inputbuf[MAXSIZE];
	char cmd[8];
	unsigned char data[MAXSIZE];
	int i, usrlen, datalen;
	int temptype, dtype = 1;		// 1 string , 0 hex

	char server_ipaddr[24] = {0};
	GET_SERVER_IP(server_ipaddr);
	GET_SERVER_IP(target_ipaddr);

	zidentify_no_t zidentify_no;
	cidentify_no_t cidentify_no;
	
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

#ifdef TRANS_UDP_SERVICE
		if(!strncmp("gp", cmd, 2) && strlen(cmd)==2)
		{
			incode_ctoxs(zidentify_no, data, 16);
			incode_ctoxs(cidentify_no, "1122334455667788", 16);
			send_gp_udp_request(server_ipaddr, TRINFO_NONE, zidentify_no, cidentify_no, NULL, 0);
		}
		else if(!strncmp("dc", cmd, 2) && strlen(cmd)==2)
		{
			send_dc_udp_request(target_ipaddr, TRINFO_DATA, 
				zidentify_no, cidentify_no, data, datalen);
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

void set_target_ip(char *ipaddr, int len)
{
	if(ipaddr == NULL)
	{
		return;
	}
	
	memset(target_ipaddr, 0, sizeof(target_ipaddr));
	memcpy(target_ipaddr, ipaddr, len);
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

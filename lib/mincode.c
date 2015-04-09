/*
 * incode.c
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
#include "mincode.h"

char ctox(char src)
{
	char temp = 0;

	if(src>='0' && src<='9')
		temp = src-'0';
	else if(src>='a' && src<='f')
		temp = src-'a'+10;
	else if(src>='A' && src<='F')
		temp = src-'A'+10;

	return temp;
}

char atox(char *src, int len)
{
	char temp=0, i=0, length;
	length = len;

	while(length--)
	{
		temp = ctox(*(src+i)) + (temp<<4);
		i++;
	}

	return temp;
}
 
void incode_ctoxs(char *dest ,char *src, int len)
{
	int i, temp;
	if(len<2 || dest==NULL || src==NULL)
	{
		return;
	}

	for(i=0; i<(len>>1); i++)
	{
		temp = *(src+(i<<1));
		dest[i] = (ctox(temp)<<4);
		
		temp = *(src+(i<<1)+1);
		dest[i] += ctox(temp);
	}
	 
}

void incode_xtocs(char *dest , char *src, int len)
{
    int i, temp;
	if(len<1 || dest==NULL || src==NULL)
	{
		return;
	}

	for(i=0; i<len; i++)
	{
		temp = (*(src+i)>>4);
		if(temp < 0xA)
		{
			dest[(i<<1)] = temp + '0';	
		}
		else
		{
			dest[(i<<1)] = temp - 0xA + 'A';	
		}
		
		temp = (*(src+i)&0x0F);
		if(temp < 0xA)
		{
			dest[(i<<1)+1] = temp + '0';	
		}
		else
		{
			dest[(i<<1)+1] = temp - 0xA + 'A';	
		}
	}
}


void incode_ctox16(short *dest, char *src)
{
	char dsts[2];
	incode_ctoxs(dsts, src, 4);
	*dest = dsts[0]<<8;
	*dest += dsts[1];
}

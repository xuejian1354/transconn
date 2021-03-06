/*
 * incode.h
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
 
#ifndef __INCODE_H__
#define __INCODE_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

unsigned char ctox(char src);
unsigned char atox(char *src, int len);
void incode_ctoxs(unsigned char *dest , char *src, int len);
void incode_xtocs(char *dest , unsigned char *src, int len);
void incode_ctox16(unsigned short *dest, char *src);
void incode_xtoc16(char *dest, unsigned short src);
void incode_ctox32(unsigned int *dest, char *src);
void incode_xtoc32(char *dest, unsigned int src);
unsigned int gen_rand(unsigned char *seed);

#ifdef __cplusplus
}
#endif

#endif //  __INCODE_H __

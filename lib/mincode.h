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

char ctox(char src);
char atox(char *src, int len);
void incode_ctoxs(char *dest , char *src, int len);
void incode_xtocs(char *dest , char *src, int len);
void incode_ctox16(short *dest, char *src);

#endif //  __INCODE_H __

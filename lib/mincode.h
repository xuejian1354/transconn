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

#include <mconfig.h>

uint8 ctox(uint8 src);
uint8 atox(uint8 *src, uint8 len);
void incode_ctoxs(uint8 *dest ,uint8 *src, uint8 len);
void incode_xtocs(uint8 *dest , uint8 *src,uint8 len);
void incode_ctox16(uint16 *dest, uint8 *src);

#endif //  __INCODE_H __

/*
 * serial.h
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
#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <mconfig.h>

#define SERIAL_MAX_LEN 128

#ifdef SERIAL_SUPPORT
int serial_open(char *dev);
int set_serial_params(int fd, 
	uint32 speed, uint8 databit, uint8 stopbit, uint8 parity);
int serial_initial(char *dev);
int serial_write(char *data, int datalen);
void *uart_read_func(void *p);
int serial_send_frame(int fd, 
	uint8 frame_type, uint8 id, uint8 *data, uint8 data_len);
#endif

#endif //__SERIAL_H__

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

#include <services/globals.h>

#ifdef SERIAL_SUPPORT

#define SERIAL_MAX_LEN 128

int serial_open(char *dev);
int set_serial_params(int fd, 
	uint32 speed, uint8 databit, uint8 stopbit, uint8 parity);
#ifdef UART_COMMBY_SOCKET
int get_uart_refd();
int get_reser_fd();
int get_reser_accept(int fd);
#endif
int serial_init(char *dev);
int serial_write(char *data, int datalen);
void *uart_read_func(void *p);

#endif
#endif //__SERIAL_H__

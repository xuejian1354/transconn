/*
 * perznet.c
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
#include <module/serial.h>

#define ZDEVICE_CMD_PERMIT		"permit"
#define ZDEVICE_CMD_FORBID		"forbid"

#define ZDEVICE_ADD_PERMIT		"D:/CJ/000001:O\r\n"
#define ZDEVICE_ADD_FORBID		"D:/CJ/000000:O\r\n"
#define ZDEVICE_ADD_FRSIZE		16

#define WARNS()	printf("[e.g.]\n./perznet /dev/ttyS1 [permit|forbid] 0000\n")

int main(int argc, char **argv)
{
	char serial_dev[16] = "/dev/ttyS1";
	char *test_no = "1234567890ABCDEF";
	char cmd[8] = ZDEVICE_CMD_PERMIT;
	char zaddr[5] = "0000";
	char sen_format[64] = {0};

	if(argc > 1)
	{
		if(!strncmp(argv[1], "transadd", 8))
		{
#ifdef SERIAL_SUPPORT
			if(serial_init(serial_dev) < 0)			//initial serial port
			{
				WARNS();
				return -1;
			}
#endif
			goto transadd;
		}
		
		memset(serial_dev, 0, sizeof(serial_dev));
		sprintf(serial_dev, "%s", argv[1]);
	}

	if(argc >2)
	{
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "%s", argv[2]);
	}

	if(argc > 3)
	{
		memset(zaddr, 0, sizeof(zaddr));
		sprintf(zaddr, "%s", argv[3]);
	}

	if(argc > 4)
	{
		printf("%s()%d : error params\n", __FUNCTION__, __LINE__);
		WARNS();
		return -1;
	}
	
#ifdef SERIAL_SUPPORT
	if(serial_init(serial_dev) < 0)			//initial serial port
	{
		WARNS();
		return -1;
	}
#endif
	
	if(!strncmp(cmd, ZDEVICE_CMD_PERMIT, 6))
	{
		sprintf(sen_format, "D:/CJ/%s01:O\r\n", zaddr);
		serial_write(sen_format, ZDEVICE_ADD_FRSIZE);
		printf("Permit add zdevice net, znet:%s\n%s\n", zaddr, sen_format);
		PRINT_HEX(sen_format, ZDEVICE_ADD_FRSIZE);
	}
	else if(!strncmp(cmd, ZDEVICE_CMD_FORBID, 6))
	{
		sprintf(sen_format, "D:/CJ/%s00:O\r\n", zaddr);
		serial_write(sen_format, ZDEVICE_ADD_FRSIZE);
		printf("Forbid add zdevice net, znet:%s\n%s\n", zaddr, sen_format);
	}
	else
	{
		printf("No found command \"%s\"\n", cmd);
		WARNS();
	}

	return 0;

transadd:
	sprintf(sen_format, "DC:%s%s19D:/CJ/000001:O\r\n:O\r\n", test_no, test_no);
	serial_write(sen_format, 57);
	printf("Permit add zdevice net\n%s\n", sen_format);
}


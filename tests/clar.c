/*
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <services/globals.h>


#define WARNS(n)  \
st(  \
	printf("Usage:%s <cmd> <devsn> [ctrl]\n", n);  \
	printf("cmd:\n");  \
	printf("\trefresh\n");  \
	printf("\tctrl\n");  \
)

int main(int argc, char **argv)
{
	int fd, ch;
	struct sockaddr_in u_addr, s_addr;
	char buf[256] = {0};

	while((ch = getopt(argc, argv, "h")) != -1)
    {
		switch(ch)
		{
		case 'h':
			WARNS(argv[0]);
			return 1;
		}
	}

	if(argc < 3)
	{
		printf("Unrecognize arguments.\n");
		WARNS(argv[0]);
		return -1;
	}

	if(!strcmp(argv[1], "refresh"))
	{
		sprintf(buf, "%s%s", argv[1], argv[2]);
		printf("%s  %s\n", argv[1], argv[2]);
	}
	else if(!strcmp(argv[1], "ctrl"))
	{
		if(argc < 4)
		{
			printf("Unrecognize arguments.\n");
			WARNS(argv[0]);
			return -1;
		}

		sprintf(buf, "%s%s%s", argv[1], argv[2], argv[3]);
		printf("%s  %s  %s\n", argv[1], argv[2], argv[3]);
	}
	else
	{
		printf("Unrecognize arguments.\n");
		WARNS(argv[0]);
		return -1;
	}

	if((fd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("udp socket fail");
		return -1;
	}

	u_addr.sin_family = PF_INET;
	u_addr.sin_port = htons(DE_UDP_CTRL_PORT);
	u_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (bind(fd, (struct sockaddr *)&u_addr, sizeof(struct sockaddr)) < 0)
	{
		perror("bind udp ip fail");
		return -1;
	}

	s_addr = u_addr;
	//s_addr.sin_family = PF_INET;
	s_addr.sin_port = htons(TRANS_UDP_PORT);
	//s_addr.sin_addr.s_addr = inet_addr("172.0.0.1"); //htonl(INADDR_ANY);

	sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&s_addr, sizeof(struct sockaddr));

	close(fd);
}


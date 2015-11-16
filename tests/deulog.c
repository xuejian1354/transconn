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
#include <pthread.h>
#include <services/globals.h>

int udp_init();
void *udp_read_func(void *p);
void udp_sendto(char *data, int len);


static int fd;
static struct sockaddr_in u_addr;
static int mudp_port = TRANS_UDP_PORT;

int main(int argc, char **argv)
{
	if(argc == 2)
	{
		mudp_port = atoi(argv[1]);
	}
	printf("UDP: m_port=%d, s_port=%d\n", DE_UDP_PORT, mudp_port);
	
	if (udp_init() < 0)
	{
		return -1;
	}

	while(1)
	{
		char buf[32] = {0};
		fgets(buf, sizeof(buf), stdin);

		int i = 0;
		while(buf[i]==' ' || buf[i]=='\n')
		{
			i++;
		}

		if(!strncmp(buf+i, "clear", 5))
		{
			system("clear");
			continue;
		}
		
		if(strlen(buf+i) == 0)
		{
			continue;
		}
		
		udp_sendto(buf+i, strlen(buf+i));
	}
}

int udp_init()
{	
	if((fd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("udp socket fail");
		return -1;
	}

	u_addr.sin_family = PF_INET;
	u_addr.sin_port = htons(DE_UDP_PORT);
	u_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (bind(fd, (struct sockaddr *)&u_addr, sizeof(struct sockaddr)) < 0)
	{
		perror("bind udp ip fail");
		return -1;
	}

	pthread_t udpRead;
	pthread_create(&udpRead, NULL, udp_read_func, NULL);
}

void *udp_read_func(void *p)
{
	while(1)
	{
		int nbytes;
		char buf[0x4000] = {0};

		nbytes = recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL);
		printf("%s", buf);
	}
}

void udp_sendto(char *data, int len)
{
	struct sockaddr_in s_addr = u_addr;
	
	//s_addr.sin_family = PF_INET;
	s_addr.sin_port = htons(mudp_port);
	//s_addr.sin_addr.s_addr = inet_addr("172.0.0.1"); //htonl(INADDR_ANY);
	
	sendto(fd, data, len, 0, (struct sockaddr *)&s_addr, sizeof(struct sockaddr));
}

/*
 * smain.c
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
#include <module/balancer.h>
#include <module/netapi.h>
#include <module/dbopt.h>

int main(int argc, char **argv)
{
#ifdef READ_CONF_FILE
	if(conf_read_from_file() < 0)
	{
		return -1;
	}
#endif

	if(start_params(argc, argv) != 0)
	{
		return 1;
	}

#ifdef DB_API_SUPPORT
	if(sql_init() < 0)
	{
		sql_release();
		return -1;
	}
	//sql_release();
#endif

#ifdef THREAD_POOL_SUPPORT
	if (tpool_create(TRANS_SERVER_THREAD_MAX_NUM) < 0)
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
	if (socket_udp_service_init(get_udp_port()) < 0)
	{
		return -1;
	}
#endif

#ifdef TRANS_TCP_SERVER
	if (socket_tcp_server_init(get_tcp_port()) < 0)
	{
		return -1;
	}
#endif

	if(mach_init() < 0)
	{
		return -1;
	}
	
	event_init();

	while(1)
	{
#ifdef SELECT_SUPPORT
		select_listen();
#endif
	}

end:	
	printf("%s()%d : end!(#>_<#)\n", __FUNCTION__, __LINE__);
	return 0;
}

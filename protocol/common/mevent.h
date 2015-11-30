/*
 * mevent.h
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
 #ifndef __MEVENT_H__
 #define __MEVENT_H__

 #include <services/globals.h>
 #include <services/etimer.h>
 #include <protocol/protocol.h>

#ifdef TIMER_SUPPORT
#ifdef COMM_CLIENT
void gateway_init();
void set_refresh_check();
void set_heartbeat_check(int interval);
void set_zdev_check(uint16 net_addr);
void set_cli_check(cli_info_t *p_cli);
#endif

#ifdef COMM_SERVER
void set_gateway_check(zidentify_no_t gw_no, int rand);
#endif

void set_mevent(int id, 
	timer_callback_t event_callback, timer_event_param_t *param);
#endif
 
 #endif  //__MEVENT_H__

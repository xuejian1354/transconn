/*
 * dbopt.h
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
#ifndef __DBOPT_H__
#define __DBOPT_H__

#include <services/globals.h>
#include <protocol/protocol.h>
#include <mysql/mysql.h>

int Sql_AddZdev(dev_info_t *m_dev);
dev_info_t *Sql_QueryZdev(zidentify_no_t zidentity_no);
int Sql_DelZdev(zidentify_no_t zidentity_no);

int Sql_AddGateway(gw_info_t *m_gw);
gw_info_t *Sql_QueryGateway(zidentify_no_t gw_no);
int Sql_DelGateway(zidentify_no_t gw_no);

void Sql_Test();

#endif	//__DBOPT_H__
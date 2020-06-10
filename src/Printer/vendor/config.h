/*
 * Copyright (C) 2019 ~ 2019 Uniontech Software Co., Ltd.
 *
 * Author:     Wei xie <xiewei@deepin.com>
 *
 * Maintainer: Wei xie  <xiewei@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

#define UTF8_T_S(str) QString::fromUtf8(str)
#define STR_T_UTF8(str) str.toUtf8().data()
#define UNUSED(x) (void)x

#define MAX_RETRY 30 //最大重试次数
#define MAX_SEQUENCE_NUM 91
#define MAX_SEQUENCE_SPEED 20

#define s_linkTemplate "<a href='%1' style='text-decoration: none; color: #0066ec;'>%2</a>"

#endif // CONFIG_H

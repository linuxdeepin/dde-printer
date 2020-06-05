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

#ifndef QTCONVERT_H
#define QTCONVERT_H

#include <QStringList>
#include <QFont>
#include <QFontMetrics>

#include <stdlib.h>
#include <string.h>
#include <vector>
#include <map>
using namespace std;

enum {
    ORDER_Forward,
    ORDER_Reverse
};

#define STQ(a) QString::fromUtf8((a).c_str())

vector<string> qStringListStdVector(const QStringList &strList);

QString attrValueToQString(const string &value);

map<string, string> mapValueByIndex(const map<int, map<string, string>> &mapData, int index, int order = ORDER_Forward);

int intMapKeyByIndex(const map<int, map<string, string>> &mapData, int index, int order = ORDER_Forward);

void dumpStdMapValue(const map<string, string> &mapValue);

void geteElidedText(const QFont &font, QString &str, int maxWidth);

#endif // QTCONVERT_H

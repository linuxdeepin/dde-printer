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

#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QMap>
#include <QVariant>
#include <QLoggingCategory>
#include <QDebug>
#include <cups/adminutil.h>

//将输入字符串转为小写，分隔出字母和数字
QString normalize(const QString &strin);

//解析1284 IE
QMap<QString, QString> parseDeviceID(const QString &strId);

//替换厂商名字，统一不同写法
//如果传入len，len返回被替换名字的长度
QString replaceMakeName(QString &make_and_model, int *len);

//移除model中的make信息
void removeMakeInModel(QString &strMake, QString &strModel);

//解析make-and-model字段中的厂商和型号信息
void ppdMakeModelSplit(const QString &strMakeAndModel, QString &strMake, QString &strModel);

int shellCmd(const QString &cmd, QString &out, QString &strErr, int timeout = 30000);

//获取ipp请求的值
QVariant ipp_attribute_value(ipp_attribute_t *attr, int i);

QString getHostFromUri(const QString &strUri);

//通过空格分割字符串，在""内的空格不算
QStringList splitStdoutString(const QString &str);

//获取打印机PPD文件路径
QString getPrinterPPD(const char *name);

//获取打印机uri
QString getPrinterUri(const char *name);

bool isPackageExists(const QString &package);

QString reslovedHost(const QString &strHost);

QString getPrinterNameFromUri(const QString &uri);

QString toNormalName(const QString &name);

#endif // COMMON_H

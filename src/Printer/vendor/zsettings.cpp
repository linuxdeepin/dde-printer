/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
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

#include "zsettings.h"
#include "config.h"

#include <QSettings>
#include <QFile>

#define VERSION         "1.2.0"
#define CLIENT_CODE     "godfather"
#define HOST_PORT       80
#define SERVER_ADDR     "printer.deepin.com"
#define OS_VERSION      "eagle"

zSettings* zSettings::getInstance()
{
    QString strHome = getenv("HOME");
    QString configFile = strHome + "/.config/dde-printer.ini";
    static zSettings instance(configFile);

    return &instance;
}


zSettings::zSettings(const QString &fileName)
    : QSettings(fileName, QSettings::NativeFormat)
{

}

const QString zSettings::getClientVersion()
{
    return value("ClientVersion", VERSION).toString();
}

const QString zSettings::getClientCode()
{
    return value("ClientCode", CLIENT_CODE).toString();
}

const QString zSettings::getHostName()
{
    return value("HostName", SERVER_ADDR).toString();
}

unsigned short zSettings::getHostPort()
{
    return static_cast<unsigned short>(value("HostPort", HOST_PORT).toInt());
}

const QString zSettings::getLogRules()
{
    return value("logRules", "*.debug=false").toString();
}

const QString zSettings::getOSVersion()
{
    return value("OSVersion", OS_VERSION).toString();
}

int zSettings::getSubscriptionId()
{
    return value("SubscriptionId", -1).toInt();
}

void zSettings::setSubscriptionId(int id)
{
    setValue("SubscriptionId", id);
    sync();
}

int zSettings::getSequenceNumber()
{
    return value("SequenceNumber", 0).toInt();
}

void zSettings::setSequenceNumber(int number)
{
    setValue("SequenceNumber", number);
    sync();
}

/*
 * Copyright (C) 2019 ~ 2020 Uniontech Software Co., Ltd.
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

#ifndef ZSETTINGS_H
#define ZSETTINGS_H

#include <QSettings>

class zSettings : public QSettings
{
public:
    static zSettings *getInstance();

    const QString getClientVersion();
    const QString getClientCode();
    const QString getHostName();
    unsigned short getHostPort();
    const QString getLogRules();
    const QString getOSVersion();
    const QString getDriverPlatformUrl();
    const QString getSystemArch(); 
    int getSubscriptionId();
    void setSubscriptionId(int id);

    int getSequenceNumber();
    void setSequenceNumber(int number);

    const QString getCupsServerHost();
    int getCupsServerPort();
    int getCupsServerEncryption();
    QString getSysInfo();
protected:
    zSettings(const QString &fileName);
    ~zSettings();
};

#define g_Settings zSettings::getInstance()

#endif // ZSETTINGS_H

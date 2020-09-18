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

#include "zsettings.h"
#include "config.h"

#include <DSysInfo>

#include <QSettings>
#include <QFile>
#include <QLocale>
#include <QDebug>

#include <sys/utsname.h>
#include <cups/cups.h>

#define VERSION         "1.2.0"
#define CLIENT_CODE     "godfather"
#define HOST_PORT       80
#define SERVER_ADDR     "printer.deepin.com"
#define OS_VERSION      "eagle"
static QMap<int, QString> DeepinTypeStrMap({{0, "unknown"}, {1, "apricot"}, {2, "eagle"}, {3, "fou"}, {4, "plum"}});

QString sysArch()
{
    struct utsname name {};

    if (uname(&name) == -1) {
        return "";
    }

    auto machine = QString::fromLatin1(name.machine);

    // map arch
    auto archMap = QMap<QString, QString> {
        {"x86_64", "x86"},
        {"i386", "x86"},
        {"i686", "x86"},
        {"mips64", "mips64"},
        {"aarch64", "aarch64"}
    };
    qInfo() << machine;
    return archMap[machine];
}

zSettings *zSettings::getInstance()
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

zSettings::~zSettings()
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
    QString defaultVersion = DeepinTypeStrMap.value(DTK_CORE_NAMESPACE::DSysInfo::deepinType(), OS_VERSION);
    QString archName = sysArch();

    qInfo() << QLocale::languageToString(QLocale::system().language());

    if (QLocale::system().language() == QLocale::Chinese && !archName.isEmpty())
        defaultVersion += "-"  + archName;
    else
        defaultVersion = "";

    return value("OSVersion", defaultVersion).toString();
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

/*
 * 返回默认本地服务器host port encryption
 * 提供切换服务器界面之后通过cupsSetServer()设置当前服务器host
 * cupsSetEncryption()设置加密方式
 * 不需要存储在本地配置中
*/
const QString zSettings::getCupsServerHost()
{
    return value("CupsServerHost", cupsServer()).toString();
}

int zSettings::getCupsServerPort()
{
    return value("CupsServerPort", ippPort()).toInt();
}
//加密配置只影响当前线程，如果要修改加密配置需要在每个线程都掉用cupsSetEncryption
int zSettings::getCupsServerEncryption()
{
    return value("CupsServerEncryption", cupsEncryption()).toInt();
}


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

#include "zdevicemanager.h"
#include "cupsattrnames.h"
#include "cupsconnection.h"
#include "qtconvert.h"
#include "common.h"
#include "config.h"
#include "dprintermanager.h"

#include <QProcess>
#include <QRegularExpression>
#include <QStringList>
#include <QTcpSocket>
#include <QUrl>

#include <libsmbclient.h>

#include <limits.h>
#include <stdlib.h>

bool        g_smbAuth;
QString     g_smbworkgroup;
QString     g_smbuser;
QString     g_smbpassword;

#define ERR_SocketBase      1000
#define SOCKET_Timeout      3000

typedef struct tagBackendSchemes{
   const char* includeSchemes;
   const char* excludeSchemes;
}TBackendSchemes;

TBackendSchemes g_backendSchemes[] = {{"hp", CUPS_EXCLUDE_NONE},
                                     {"usb", CUPS_EXCLUDE_NONE},
                                      {"snmp", CUPS_EXCLUDE_NONE},
                                      {"smfpnetdiscovery", CUPS_EXCLUDE_NONE},
                                     {CUPS_INCLUDE_ALL, "cups-brf,dcp,parallel,serial"}};

static void smb_auth_func(SMBCCTX *c,
        const char *srv,
        const char *shr,
        char *wg, int wglen,
        char *un, int unlen,
        char *pw, int pwlen)
{
    UNUSED(c);
    UNUSED(srv);
    UNUSED(shr);
    if (g_smbworkgroup.isEmpty()) {
        g_smbworkgroup = wg;
        strncpy(wg, "WORKGROUP", wglen);
    }
    else
        strncpy(wg, g_smbworkgroup.toUtf8().constData(), wglen);

    if (g_smbuser.isEmpty()) {
        g_smbuser = un;
        /* if user is empty, wu use nobody instead,
         * otherwise smbspool will use kerberos authentication */
        strncpy(un, "nobody", unlen);
    }
    else
        strncpy(un, g_smbuser.toUtf8().constData(), unlen);

    if (g_smbpassword.isEmpty())
        strncpy(pw, " ", pwlen);
    else
        strncpy(pw, g_smbpassword.toUtf8().constData(), pwlen);
}

ReflushDevicesTask::ReflushDevicesTask(int id, QObject* parent)
    :TaskInterface (id, parent)
{}

QList<TDeviceInfo> ReflushDevicesTask::getResult()
{
    QMutexLocker locker(&m_mutex);

    return m_devices;
}

int ReflushDevicesTask::addDevice(const TDeviceInfo &dev)
{
    {
        QMutexLocker locker(&m_mutex);

        m_devices.append(dev);
    }

    emit signalStatus(m_iTaskId, TStat_Update);

    return 0;
}

void ReflushDevicesTask::clearDevices()
{
    QMutexLocker locker(&m_mutex);

    m_devices.clear();
}

ReflushDevicesByBackendTask::ReflushDevicesByBackendTask(int id, QObject* parent)
    :ReflushDevicesTask(id, parent)
{}

int ReflushDevicesByBackendTask::mergeDevice(TDeviceInfo &device, const char * backend)
{
    QString uri = device.uriList[0];
    //排除重复的URI
    for (auto item : getResult()) {
        if (item.uriList.contains(uri)) {
            qInfo() << "remove same uri";
            return -1;
        }
    }

    //合并hplip和usb后端发现的同一台打印机
    if (uri.startsWith("usb:") || uri.startsWith("hp:")) {
        QRegularExpression re("serial=([^&]*)");
        QRegularExpressionMatch match = re.match(uri);
        if (match.hasMatch()) {
            QString serial = match.captured(1).toLower();
            device.serial = serial;
            for (auto &item : getResult())
                if (!device.strClass.compare(item.strClass) &&
                        (item.uriList[0].startsWith("usb:") ||
                        item.uriList[0].startsWith("hp:")) &&
                        !serial.compare(item.serial)) {
                    item.uriList << uri;
                    qInfo() << "merge uri " << item.uriList;
                    return 1;
                }
        }
    }

    //排除三星后端找到重复设备的情况
    if (backend && 0 == strcmp(backend, "smfpnetdiscovery")) {
        for (auto &item : getResult()) {
            if (item.strInfo == device.strInfo) {
                if (item.uriList[0].startsWith("ipp:")) {
                    item.uriList.clear();
                    item.uriList = device.uriList;
                    qInfo() << item.strInfo + "change uri to" + uri;
                }
                qInfo() << "remove same device use samsung_schemes";
                return 1;
            }
        }
    }

    return 0;
}

int ReflushDevicesByBackendTask::addDevices(const map<string, map<string, string>> &devs, const char * backend)
{
    map<string, map<string, string>>::const_iterator itmap;

    for(itmap=devs.begin();itmap != devs.end();itmap++){
        TDeviceInfo info;
        QString uri = STQ(itmap->first);
        map<string, string> infomap = itmap->second;

        if (!uri.contains(':'))
            continue;

        dumpStdMapValue(infomap);

        QRegularExpression re_ipv6("([\\da-fA-F]{1,4}:){6}((25[0-5]|2[0-4]\\d|[01]?\\d\\d?)\\.){3}(25[0-5]|2[0-4]\\d|[01]?\\d\\d?)"
                                   "|::([\\da-fA-F]{1,4}:){0,4}((25[0-5]|2[0-4]\\d|[01]?\\d\\d?)\\.){3}(25[0-5]|2[0-4]\\d|[01]?\\d\\d?)"
                                   "|([\\da-fA-F]{1,4}:):([\\da-fA-F]{1,4}:){0,3}((25[0-5]|2[0-4]\\d|[01]?\\d\\d?)\\.){3}(25[0-5]|2[0-4]\\d"
                                   "|[01]?\\d\\d?)|([\\da-fA-F]{1,4}:){2}:([\\da-fA-F]{1,4}:){0,2}((25[0-5]|2[0-4]\\d|[01]?\\d\\d?)\\.){3}(25[0-5]"
                                   "|2[0-4]\\d|[01]?\\d\\d?)|([\\da-fA-F]{1,4}:){3}:([\\da-fA-F]{1,4}:){0,1}((25[0-5]|2[0-4]\\d|[01]?\\d\\d?)\\.){3}(25[0-5]"
                                   "|2[0-4]\\d|[01]?\\d\\d?)|([\\da-fA-F]{1,4}:){4}:((25[0-5]|2[0-4]\\d|[01]?\\d\\d?)\\.){3}(25[0-5]|2[0-4]\\d|[01]?\\d\\d?)"
                                   "|([\\da-fA-F]{1,4}:){7}[\\da-fA-F]{1,4}|:((:[\\da-fA-F]{1,4}){1,6}|:)|[\\da-fA-F]{1,4}:((:[\\da-fA-F]{1,4}){1,5}|:)"
                                   "|([\\da-fA-F]{1,4}:){2}((:[\\da-fA-F]{1,4}){1,4}|:)|([\\da-fA-F]{1,4}:){3}((:[\\da-fA-F]{1,4}){1,3}|:)"
                                   "|([\\da-fA-F]{1,4}:){4}((:[\\da-fA-F]{1,4}){1,2}|:)|([\\da-fA-F]{1,4}:){5}:([\\da-fA-F]{1,4})?|([\\da-fA-F]{1,4}:){6}:");
        if (re_ipv6.match(uri).hasMatch()) {
            qInfo() << "Unspport ipv6 uri";
            continue;
        }

        info.uriList << uri;
        info.strClass = attrValueToQString(infomap[CUPS_DEV_CLASS]);
        info.strInfo = attrValueToQString(infomap[CUPS_DEV_INFO]);
        info.strMakeAndModel = attrValueToQString(infomap[CUPS_DEV_MAKE_MODE]);
        info.strDeviceId = attrValueToQString(infomap[CUPS_DEV_ID]);
        info.strLocation = attrValueToQString(infomap[CUPS_DEV_LOCATION]);
        info.iType = InfoFrom_Detect;

        if (0 != mergeDevice(info, backend))
            continue;

        qInfo() << QString("Add printer %1, by:%2").arg(info.toString()).arg(backend?backend:"other");
        addDevice(info);
    }

    return 0;
}

int ReflushDevicesByBackendTask::doWork()
{
    int sechCount = sizeof(g_backendSchemes)/sizeof(g_backendSchemes[0]);
    int snmpCount = 0;

    clearDevices();

    for (int i=0;i<sechCount;i++) {
        const char *inSech = g_backendSchemes[i].includeSchemes;
        vector<string> inSechemes, exSechemes;
        map<string, map<string, string>> devs;

        //snmp找到设备的情况不用三星的后端查找设备
        if (snmpCount > 0 && inSech
            && 0 == strcmp(inSech, "smfpnetdiscovery")) {
            continue;
        }

        int lastPrinterCount = getResult().count();
        if (inSech == CUPS_INCLUDE_ALL) {
            QStringList exlist = QString(g_backendSchemes[i].excludeSchemes).split(",");

            //CUPS_INCLUDE_ALL的情况排除之前已经查找的后端
            for(int j=0;j<i;j++) {
                exlist.append(g_backendSchemes[j].includeSchemes);
            }

            exSechemes = qStringListStdVector(exlist);
            qDebug() << "Get devices by all other backends: " << exlist;
        } else {
            inSechemes.push_back(inSech);
            qDebug() << "Get devices by" << inSech;
        }

        try {
            devs = g_cupsConnection->getDevices(&exSechemes, &inSechemes, 0, CUPS_TIMEOUT_DEFAULT);
        }catch(const std::exception &ex) {
            qWarning() << "Got execpt: " << QString::fromUtf8(ex.what());
            continue;
        };

        if (m_bQuit) return 0;

        addDevices(devs, inSech);

        if (inSech && 0 == strcmp(inSech, "snmp"))
            snmpCount = getResult().count() - lastPrinterCount;
    }

    qInfo() << QString("Got %1 devices").arg(getResult().count());

    return 0;
}

ReflushDevicesByHostTask::ReflushDevicesByHostTask(const QString &strHost, int id, QObject* parent)
    :ReflushDevicesTask (id, parent)
{
    m_strHost = strHost;
}

int ReflushDevicesByHostTask::probe_snmp(const QString &strHost)
{
    qDebug() << strHost;
    QString strRet, strErr;
    int iRet = shellCmd(QString("/usr/lib/cups/backend/snmp ") + strHost, strRet, strErr);
    if(0 == iRet)
    {
        QStringList retList = strRet.split("\n");
        foreach (QString str, retList) {
            if (str.isEmpty()) continue;

            qDebug() << "Got snmp " << str;
            QStringList list = splitStdoutString(str);
            if (list.count() < 4) return -2;

            TDeviceInfo info;
            info.iType = InfoFrom_Detect;
            info.strClass = list[0];
            info.uriList<<list[1];
            info.strMakeAndModel = list[2];
            info.strInfo = list[3];
            if (list.count() > 4)
                info.strDeviceId = list[4];
            if (list.count() > 5)
                info.strLocation = list[5];

            addDevice(info);
        }
    }

    return iRet;
}

int ReflushDevicesByHostTask::probe_hplip(const QString &strHost)
{
    QString strRet;
    QString strErr;
    int iRet = shellCmd(QString("hp-makeuri -c ") + strHost, strRet, strErr);
    strRet = strRet.trimmed();
    if(0 == iRet && strRet.contains(":"))
    {
        //TODO get uri and info
        TDeviceInfo info;
        info.uriList<<strRet;
        info.iType = InfoFrom_Detect;
        addDevice(info);
    }

    return iRet;
}

int ReflushDevicesByHostTask::probe_jetdirect(const QString &strHost)
{
    qDebug() << "probe_jetdirect" << strHost;
    QTcpSocket socket;
    socket.connectToHost(strHost, 9100);
    if (socket.waitForConnected(SOCKET_Timeout))
    {
        TDeviceInfo info;
        info.uriList << QString("socket://%1:%2").arg(strHost).arg(9100);
        info.iType = InfoFrom_Detect;
        qDebug() << info.uriList;
        addDevice(info);
        return 0;
    }

    qDebug() << QString("Connect appsocket %1 failed, err: (%2) %3").arg(strHost).arg(socket.error()).arg(socket.errorString());
    return -1;
}

int ReflushDevicesByHostTask::probe_ipp(const QString &strHost)
{
    map<string, map<string,string>> printersMap;
    map<string, map<string,string>>::iterator itPrinters;
    Connection c;

    try {
        if (0 != c.init(strHost.toUtf8().data(), 0, 0)) {
            qWarning() << "Unable to connect " << strHost;
            return -1;
        }
        printersMap = c.getPrinters();
    }catch(const std::exception &ex) {
        qWarning() << "Got execpt: " << QString::fromUtf8(ex.what());
        return -2;
    };

    for (itPrinters=printersMap.begin();itPrinters!=printersMap.end();itPrinters++) {
        TDeviceInfo info;
        map<string, string> infomap = itPrinters->second;

        dumpStdMapValue(infomap);
        info.strName = STQ(itPrinters->first);
        info.strInfo = attrValueToQString(infomap[CUPS_OP_INFO]);
        info.strLocation = attrValueToQString(infomap[CUPS_OP_LOCATION]);
        info.strMakeAndModel = attrValueToQString(infomap[CUPS_OP_MAKE_MODEL]);
        info.uriList << attrValueToQString(infomap[CUPS_OP_URI_SUP]);

        addDevice(info);
    }

    return 0;
}

#define LPD_MAX 1
int ReflushDevicesByHostTask::probe_lpd(const QString &strHost)
{
    qDebug() << "probe_lpd" << strHost;
    QTcpSocket socket;
    socket.connectToHost(strHost, 515);
    if (socket.waitForConnected(SOCKET_Timeout)){
        TDeviceInfo info;
        info.iType = InfoFrom_Detect;
        info.uriList << QString("lpd://%1/%2").arg(strHost).arg("Unknown");
        addDevice(info);
        return 0;
    }

    qDebug() << QString("Connect appsocket %1 failed, err: (%2) %3").arg(strHost).arg(socket.error()).arg(socket.errorString());
    return -1;
}

int ReflushDevicesByHostTask::probe_smb(const QString &strHost)
{
    qDebug() << "probe_smb" << strHost;
    int ret = 0;
    int try_again = 0;
    QString uri = "smb://";
    if (!strHost.isEmpty())
        uri += QUrl(strHost).toEncoded();
    if(!uri.endsWith('/'))
        uri += '/';
    QByteArray uri_utf8 = uri.toUtf8();

    SMBCCTX *ctx = nullptr;
    SMBCFILE *fd = nullptr;
    if ((ctx = smbc_new_context()) == nullptr) {
        ret = -1;
        goto done;
    }

    smbc_setFunctionAuthDataWithContext(ctx, smb_auth_func);

    if (smbc_init_context(ctx) == nullptr) {
        ret =  -2;
        goto done;
    }

    g_smbAuth = false;
    fd = smbc_getFunctionOpendir(ctx)(ctx, uri_utf8.constData());
    while (!fd) {
        int last = try_again;
        qDebug() << "error: " << errno;
        //if (errno != EACCES && errno != EPERM) {
        //    qDebug() << errno;
        //    ret = -3;
        //    goto done;
        //}
        g_smbAuth = true;
        emit signalSmbPassWord(try_again, strHost, g_smbworkgroup, g_smbuser, g_smbpassword);
        if (try_again <= last)
            goto done;

        fd = smbc_getFunctionOpendir(ctx)(ctx, uri_utf8.constData());
    }

    /* insert workgroup after smb:// */
    if (!g_smbworkgroup.isEmpty())
        uri.insert(6, QUrl(g_smbworkgroup).toEncoded() + '/');

    struct smbc_dirent *dirent;
    while ((dirent = smbc_getFunctionReaddir(ctx)(ctx, fd)) != nullptr) {
        if (dirent->smbc_type != SMBC_PRINTER_SHARE)
            continue;

        TDeviceInfo info;
        QUrl url(uri + QUrl(dirent->name).toEncoded());

        if (g_smbAuth)
        {
            url.setUserName(g_smbuser.isEmpty() ? "nobody" : g_smbuser);
            url.setPassword(g_smbpassword);
        }

        info.uriList << url.toEncoded();

        info.strMakeAndModel = dirent->comment;
        info.strInfo = info.strMakeAndModel;
        info.iType = InfoFrom_Guess;
        addDevice(info);
    }

done:
    if (fd)
        smbc_getFunctionClose(ctx)(ctx, fd);
    if (ctx)
        smbc_free_context(ctx, 1);
    qDebug() << "probe_smb ret: " << ret;
    return ret;
}

int ReflushDevicesByHostTask::doWork()
{
    clearDevices();

    m_strLastErr = reslovedHost(m_strHost);
    if (!m_strLastErr.isEmpty()) {
        return -1;
    }

    probe_snmp(m_strHost);
    if (m_bQuit) return 0;

    //如果snmp 找到设备则不用查找socket、ipp、lpd
    if (getResult().size() <= 0)
    {
        probe_jetdirect(m_strHost);
        if (m_bQuit) return 0;

        probe_ipp(m_strHost);
        if (m_bQuit) return 0;

        probe_lpd(m_strHost);
        if (m_bQuit) return 0;
    }

    probe_hplip(m_strHost);
    if (m_bQuit) return 0;

    if (getResult().size() <= 0)
        probe_smb(m_strHost);
    return 0;
}

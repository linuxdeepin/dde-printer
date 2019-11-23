/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     liurui <liurui_cm@deepin.com>
 *
 * Maintainer: liurui <liurui_cm@deepin.com>
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
#include "dprintermanager.h"
#include "dprinter.h"
#include "dprintclass.h"
#include "cupsattrnames.h"

#include <QDebug>
#include <QFile>
#include <QTextCodec>
#include <QRegularExpression>

#include <assert.h>

DPrinterManager *DPrinterManager::m_self = nullptr;

DPrinterManager::DPrinterManager()
{
    m_conn = new Connection;

    if (nullptr == m_conn) {
        assert(false && "alloc memery failed");
    }

    m_pServerSettings = m_conn->getServerSettings();
}

DPrinterManager::~DPrinterManager()
{
    if (nullptr != m_conn) {
        delete m_conn;
        m_conn = nullptr;
    }

    clearDestinationList();
}

DPrinterManager *DPrinterManager::getInstance()
{
    if (nullptr == m_self) {
        m_self = new DPrinterManager;
    }

    return m_self;
}

bool DPrinterManager::InitConnection(const char *host_uri, int port, int encryption)
{
    bool bRet = false;

    try {
        bRet = (m_conn->init(host_uri, port, encryption) == 0);
    } catch (const std::runtime_error &e) {
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
        bRet = false;
    }

    return bRet;
}

Connection *DPrinterManager::getConnection()
{
    return m_conn;
}

void DPrinterManager::setAllowedUsers(const QString &strPrinterName, const QVector<QString> strUsers)
{
    vector<string> vecTrans;

    for (int i = 0; i < strUsers.size(); i++) {
        vecTrans.push_back(strUsers[i].toStdString());
    }

    try {
        m_conn->setPrinterUsersAllowed(strPrinterName.toStdString().c_str(), &vecTrans);
    } catch (const std::runtime_error &e) {
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
    }
}

void DPrinterManager::updateDestinationList()
{
    clearDestinationList();

    try {
        map<string, map<string, string>> mapPrinters = m_conn->getPrinters();

        for (auto iter = mapPrinters.begin(); iter != mapPrinters.end(); iter++) {
            map<string, string> mapProperty = iter->second;
            string strValue = mapProperty["device-uri"];

            if (strValue.empty()) {
                continue;
            }

            strValue.erase(0, 1);

            DDestination *pDest = nullptr;

            if (0 == strValue.find("file:///dev/null")) {
                pDest = new DPrintClass(m_conn);
            } else {
                pDest = new DPrinter(m_conn);
            }

            QString strName = QString::fromStdString(iter->first);
            pDest->setName(strName);
            m_mapDests.insert(strName, pDest);
        }
    } catch (const std::runtime_error &e) {
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
    }
}

QStringList DPrinterManager::getPrintersList()
{
    QStringList printerList;

    for (auto iter = m_mapDests.begin(); iter != m_mapDests.end(); iter++) {
        if (iter.value()->getType() == PRINTER)
            printerList.push_back(iter.key());
    }

    return printerList;
}

QStringList DPrinterManager::getPrinterBaseInfoByName(QString printerName)
{
    QStringList baseInfo;
    DDestination *pDest = m_mapDests[printerName];

    if (pDest != nullptr) {
        baseInfo = pDest->getPrinterBaseInfo();
    }

    return baseInfo;
}

bool DPrinterManager::deletePrinterByName(QString PrinterName)
{
    bool bRet = false;
    try {
        m_conn->deletePrinter(PrinterName.toStdString().data(), "");
        bRet = true;
        m_mapDests.remove(PrinterName);
    } catch (const std::runtime_error &e) {
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
        bRet = false;
    }

    return bRet;
}

void DPrinterManager::setPrinterShared(QString printerName, int shared)
{
    m_conn->setPrinterShared(printerName.toStdString().data(), shared);
}

bool DPrinterManager::isPrinterShared(QString printerName)
{
    bool isShared = false;
    vector<string> printerAttrs;
    printerAttrs.push_back(CUPS_OP_ISSHAED);
    try {
        map<string, string> attrMap = m_conn->getPrinterAttributes(printerName.toStdString().data(), nullptr, &printerAttrs);
        string strShared = attrMap.at(CUPS_OP_ISSHAED);
        if (strShared.substr(0, 2) == "b1") {
            isShared = true;
        } else {
            isShared = false;
        }
    } catch (const std::runtime_error &e) {
        qWarning() << e.what();
    }
    return isShared;
}

void DPrinterManager::setPrinterEnabled(QString printerName, bool enabled)
{
    if (enabled) {
        m_conn->enablePrinter(printerName.toStdString().data(), "");
    } else {
        m_conn->disablePrinter(printerName.toStdString().data(), "");
    }
}

bool DPrinterManager::isPrinterEnabled(QString printerName)
{
    bool isEnable = false;
    vector<string> printerAttrs;
    printerAttrs.push_back(CUPS_OP_STATE);
    try {
        map<string, string> attrMap = m_conn->getPrinterAttributes(printerName.toStdString().data(), nullptr, &printerAttrs);
        if (attrMap.at(CUPS_OP_STATE).substr(0, 2) == string("i3")) {
            isEnable = true;
        } else if (attrMap.at(CUPS_OP_STATE).substr(0, 2) == string("i4")) {
            isEnable = true;
        } else {
            isEnable = false;
        }
    } catch (const std::runtime_error &e) {
        qCritical() << e.what();
    }
    return isEnable;
}

void DPrinterManager::setPrinterAcceptJob(QString printerName, bool enabled)
{
    if (enabled) {
        m_conn->acceptJobs(printerName.toStdString().data(), "");
    } else {
        m_conn->rejectJobs(printerName.toStdString().data(), "");
    }

}

bool DPrinterManager::isPrinterAcceptJob(QString printerName)
{
    bool isAcceptJobs = false;
    vector<string> printerAttrs;
    printerAttrs.push_back(CUPS_OP_TYPE);
    try {
        map<string, string> attrMap = m_conn->getPrinterAttributes(printerName.toStdString().data(), nullptr, &printerAttrs);
        int typeValue = QString(attrMap.at(CUPS_OP_TYPE).substr(1).data()).toInt();
        int result = typeValue & CUPS_PRINTER_REJECTING;
        if (result == CUPS_PRINTER_REJECTING) {
            isAcceptJobs = false;
        } else {
            isAcceptJobs = true;
        }
    } catch (const std::runtime_error &e) {
        qCritical() << e.what();
    }
    return isAcceptJobs;
}

void DPrinterManager::setPrinterDefault(QString printerName)
{
    m_conn->setDefault(printerName.toStdString().data(), "");

}

void DPrinterManager::addPrinter(const QString &printer, const QString &info, const QString &location, const QString &device, const QString &ppdfile)
{
    try {
        m_conn->addPrinter(printer.toStdString().data(), info.toStdString().data(),
                           location.toStdString().data(), device.toStdString().data(), ppdfile.toStdString().data(), nullptr, nullptr);
    } catch (const std::runtime_error &e) {
        qWarning() << e.what();
    }
}



bool DPrinterManager::isDefaultPrinter(QString PrinterName)
{
    string defaultPrinter = m_conn->getDefault();
    if (PrinterName.toStdString().compare(defaultPrinter) == 0) {
        return true;
    }
    return false;
}

DDestination *DPrinterManager::getDestinationByName(const QString &strName)
{
    if (m_mapDests.contains(strName))
        return  m_mapDests[strName];
    return nullptr;
}

bool DPrinterManager::hasUnfinishedJob()
{
    try {
        vector<string> jobAttrs{"job-id", "job-printer-uri", "job-name"};
        map<int, map<string, string>> unfinishedJobs = m_conn->getJobs(nullptr, 1, 1, 0, &jobAttrs);
        if (unfinishedJobs.size() == 0)
            return false;
        return true;
    } catch (const std::runtime_error &e) {
        qWarning() << e.what();
        return false;
    }
}

bool DPrinterManager::hasFinishedJob()
{
    try {
        vector<string> jobAttrs{"job-id", "job-printer-uri", "job-state"};
        map<int, map<string, string>> finishedJobs = m_conn->getJobs("completed", 1, 1, 0, &jobAttrs);
        if (finishedJobs.size() == 0)
            return false;
        return true;
    } catch (const std::runtime_error &e) {
        qWarning() << e.what();
        return false;
    }
}

void DPrinterManager::enableDebugLogging(bool enabled)
{
    m_pServerSettings.enableDebugLogging(enabled);
}

void DPrinterManager::enableRemoteAdmin(bool enabled)
{
    m_pServerSettings.enableRemoteAdmin(enabled);
}

void DPrinterManager::enableRemoteAny(bool enabled)
{
    m_pServerSettings.enableRemoteAny(enabled);
}

void DPrinterManager::enableSharePrinters(bool enabled)
{
    m_pServerSettings.enableSharePrinters(enabled);
}

void DPrinterManager::enableUserCancelAny(bool enabled)
{
    m_pServerSettings.enableUserCancelAny(enabled);
}

bool DPrinterManager::isDebugLoggingEnabled() const
{
    return m_pServerSettings.isDebugLoggingEnabled();
}

bool DPrinterManager::isRemoteAdminEnabled() const
{
    return m_pServerSettings.isRemoteAdminEnabled();
}

bool DPrinterManager::isRemoteAnyEnabled() const
{
    return m_pServerSettings.isRemoteAnyEnabled();
}

bool DPrinterManager::isSharePrintersEnabled() const
{
    return m_pServerSettings.isSharePrintersEnabled();
}

bool DPrinterManager::isUserCancelAnyEnabled() const
{
    return m_pServerSettings.isUserCancelAnyEnabled();
}

void DPrinterManager::commit()
{
    try {
        m_pServerSettings.commit();
    } catch (const std::runtime_error &e) {
        qWarning() << e.what();
    }
}

bool DPrinterManager::hasSamePrinter(const QString &printer)
{
    foreach (const QString &str, m_mapDests.keys()) {
        if (str == printer) {
            return true;
        }
    }
    return false;
}

QString DPrinterManager::validataName(const QString &oldPrinterName)
{
    QString newPrinterName;
    if (oldPrinterName.length() >=  128) {
        newPrinterName = oldPrinterName.left(120);
    } else {
        newPrinterName = oldPrinterName;
    }
    newPrinterName = newPrinterName.trimmed();
    newPrinterName.replace(QRegularExpression("[# /]"), "-");
    return newPrinterName;
}

void  DPrinterManager::clearDestinationList()
{
    if (m_mapDests.size() > 0) {
        for (auto iter = m_mapDests.begin(); iter != m_mapDests.end();) {
            delete iter.value();
            iter = m_mapDests.erase(iter);
        }
    }
}

void DPrinterManager::initLanguageTrans()
{
    m_translator.init();
}

QString DPrinterManager::translateLocal(const QString &strContext, const QString &strKey)
{
    QString strValue = m_translator.translateLocal(strContext, strKey);
    return strValue;
}

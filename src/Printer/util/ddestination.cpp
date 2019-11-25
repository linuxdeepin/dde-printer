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
#include "ddestination.h"
#include "cupsattrnames.h"

#include <QDebug>

#include <map>
#include <assert.h>

using namespace std;

DDestination::DDestination(Connection *con)
{
    assert(con != nullptr);
    m_pCon = con;
}

QString DDestination::getName()
{
    return m_strName;
}

void DDestination::setName(const QString &strName)
{
    m_strName = strName;
    initPrinterAttr();
}

bool DDestination::isShared()
{
    return m_isShared;
}

void DDestination::setShared(bool shared)
{
    m_isShared = shared;
}

bool DDestination::isEnabled()
{
    return m_isEnabled;
}

void DDestination::setEnabled(bool enabled)
{
    m_isEnabled = enabled;
    //设置启用和禁用之后会影响当前的打印机状态字符串信息，需要更新
    initPrinterAttr();
}

QStringList DDestination::getPrinterBaseInfo()
{
    initPrinterAttr();
    QStringList baseInfo;
    baseInfo.append(m_printerLocation);
    baseInfo.append(m_printerModel.isEmpty() ? m_printerInfo : m_printerModel);
    baseInfo.append(m_printerStatus);
    return baseInfo;
}

QString DDestination::printerInfo()
{
    QString strPrintInfo;
    vector<string> requestAttr;
    requestAttr.push_back(CUPS_OP_INFO);

    try {
        map<string, string> attr = m_pCon->getPrinterAttributes(m_strName.toStdString().c_str(), nullptr, &requestAttr);
        strPrintInfo = QString::fromStdString(attr[CUPS_OP_INFO].data());
        strPrintInfo = strPrintInfo.remove(0, 1);
    } catch (const std::runtime_error &e) {
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
    }

    return strPrintInfo;
}

void DDestination::setPrinterInfo(const QString &info)
{
    m_printerInfo = info;

    try {
        m_pCon->setPrinterInfo(m_strName.toStdString().c_str(), m_printerInfo.toStdString().c_str());
    } catch (const std::runtime_error &e) {
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
    }
}

QString DDestination::printerLocation()
{
    QString strLocation;
    vector<string> requestAttr;
    requestAttr.push_back(CUPS_OP_LOCATION);

    try {
        map<string, string> attr = m_pCon->getPrinterAttributes(m_strName.toStdString().c_str(), nullptr, &requestAttr);
        strLocation = QString::fromStdString(attr[CUPS_OP_LOCATION].data());
        strLocation = strLocation.remove(0, 1);
    } catch (const std::runtime_error &e) {
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
    }

    return strLocation;
}

void DDestination::setPrinterLocation(const QString &location)
{
    m_printerLocation = location;

    try {
        m_pCon->setPrinterLocation(m_strName.toStdString().c_str(), m_printerLocation.toStdString().c_str());
    } catch (const std::runtime_error &e) {
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
    }
}

QString DDestination::printerModel()
{
    QString strPrintModel;
    vector<string> requestAttr;
    requestAttr.push_back(CUPS_OP_MAKE_MODEL);

    try {
        if (isPpdFileBroken()) {
            map<string, string> attr = m_pCon->getPrinterAttributes(m_strName.toStdString().c_str(), nullptr, &requestAttr);
            strPrintModel = QString::fromStdString(attr[CUPS_OP_MAKE_MODEL].data());
            strPrintModel = strPrintModel.remove(0, 1);
        } else {
            strPrintModel = tr("UNKOWN");
        }
    } catch (const std::runtime_error &e) {
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
    }

    return strPrintModel;
}

bool DDestination::initPrinterPPD()
{
    return false;
}

bool DDestination::isPpdFileBroken()
{
    return true;
}

void DDestination::getPrinterInfo(QString &info, QString &location, QString &deviceURI, QString &ppdfile) const
{
    info = m_printerInfo;
    location = m_printerLocation;
    deviceURI = m_printerURI;
    ppdfile = m_ppdFile;
}


void DDestination::initPrinterAttr()
{
    vector<string> printerAttrs;
    printerAttrs.push_back(CUPS_OP_ISSHAED);
    printerAttrs.push_back(CUPS_OP_ISACCEPT);
    printerAttrs.push_back(CUPS_OP_LOCATION);
    printerAttrs.push_back(CUPS_OP_MAKE_MODEL);
    printerAttrs.push_back(CUPS_OP_STATE);
    printerAttrs.push_back(CUPS_OP_INFO);
    printerAttrs.push_back(CUPS_DEV_URI);
    printerAttrs.push_back(CUPS_OP_MAKE_MODEL);

    try {
        map<string, string> attrMap = m_pCon->getPrinterAttributes(m_strName.toStdString().data(), nullptr, &printerAttrs);
        string strShared = attrMap.at(CUPS_OP_ISSHAED);
        if (strShared.substr(0, 2) == "b1") {
            m_isShared = true;
        } else {
            m_isShared = false;
        }

        m_printerLocation = attrMap.at(CUPS_OP_LOCATION).substr(1).data();
        m_printerInfo = attrMap.at(CUPS_OP_INFO).substr(1).data();

        if (!isPpdFileBroken()) {
            m_printerModel = attrMap.at(CUPS_OP_MAKE_MODEL).substr(1).data();
        } else {
            m_printerModel = tr("UNKOWN");
        }

        m_printerURI = attrMap.at(CUPS_DEV_URI).substr(1).data();
        m_ppdFile = m_pCon->getPPD(m_strName.toStdString().data()).data();
        // 此处i3表示数据类型为int 值为3，其实是enumeration类型转换
        if (attrMap.at(CUPS_OP_STATE).substr(0, 2) == string("i3")) {
            m_printerStatus = tr("Idle");
            m_isEnabled = true;
        } else if (attrMap.at(CUPS_OP_STATE).substr(0, 2) == string("i4")) {
            m_printerStatus = tr("Printing");
            m_isEnabled = true;
        } else {
            m_printerStatus = tr("Disabled");
            m_isEnabled = false;
        }
    } catch (const std::runtime_error &e) {
        qCritical() << e.what();
    }
}

DESTTYPE DDestination::getType() const
{
    return m_type;
}

/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     shenfusheng <shenfusheng_cm@deepin.com>
 *
 * Maintainer: shenfusheng <shenfusheng_cm@deepin.com>
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
#include "refreshsnmpbackendtask.h"
#include "dprintermanager.h"
#include "dprinter.h"

#include <QDebug>

RefreshSnmpBackendTask::RefreshSnmpBackendTask(QObject *parent) : QThread(parent)
{
    m_bExit = true;
}

RefreshSnmpBackendTask::~RefreshSnmpBackendTask()
{
    if (isTaskRunning()) {
        stopTask();
        wait();
    }
}

void RefreshSnmpBackendTask::setPrinters(const QStringList &strPrinters)
{
    m_strPrinterNames = strPrinters;
}

void RefreshSnmpBackendTask::beginTask()
{
    if (!m_bExit) {
        return;
    }

    m_vecFreshNode.clear();
    DPrinterManager *pManager = DPrinterManager::getInstance();

    for (int i = 0; i < m_strPrinterNames.size(); i++) {
        DDestination *pDest = pManager->getDestinationByName(m_strPrinterNames.at(i));

        if (pDest != nullptr) {
            if (DESTTYPE::PRINTER == pDest->getType()) {
                DPrinter *pPrinter = static_cast<DPrinter *>(pDest);

                if (pPrinter != nullptr) {
                    SNMPFRESHNODE node;
                    QString strInfo;
                    pPrinter->getPrinterInfo(strInfo, node.strLoc, node.strUrl, node.strPpdName);
                    node.strName = pPrinter->getName();
                    m_vecFreshNode.push_back(node);
                }
            }
        }
    }

    start();
}

void RefreshSnmpBackendTask::stopTask()
{
    m_bExit = true;
    terminate();
}

bool RefreshSnmpBackendTask::isTaskRunning()
{
    return !m_bExit;
}

QVector<SUPPLYSDATA> RefreshSnmpBackendTask::getSupplyData(const QString &strName)
{
    QVector<SUPPLYSDATA> retData;

    if (m_mapSupplyInfo.contains(strName)) {
        retData = m_mapSupplyInfo[strName];
    }

    return retData;
}

void RefreshSnmpBackendTask::run()
{
    m_bExit = false;

    for (int i = 0; i < m_vecFreshNode.size(); i++) {

        if (m_bExit)
            break;

        SNMPFRESHNODE node = m_vecFreshNode[i];
        bool bRet = canGetSupplyMsg(node);
        QString strName = node.strName;
        emit refreshsnmpfinished(strName, bRet);
    }

    m_bExit = true;
}

bool RefreshSnmpBackendTask::canGetSupplyMsg(const SNMPFRESHNODE &node)
{
    bool bRet = false;
    QVector<SUPPLYSDATA> vecMarkInfo;

    if (node.strUrl.startsWith("socket://")) {
        time_t tm = 0;
        cupssnmp snmp;
        snmp.setIP(node.strLoc.toStdString());
        snmp.setPPDName(node.strPpdName.toStdString().c_str());
        snmp.SNMPReadSupplies();
        vector<SUPPLYSDATA> tempVec = snmp.getMarkInfo();

        for (unsigned int i = 0; i < tempVec.size(); i++) {
            vecMarkInfo.push_back(tempVec[i]);
        }
    } else if (node.strUrl.startsWith("ipp://")) {
        try {
            Connection c;
            QString strMarkerLevel;
            QString strMarkerName;
            QStringList strLevelList;
            QStringList strNameList;
            c.init(node.strLoc.toStdString().c_str(), ippPort(), 0);
            vector<string> requestAttrs;
            requestAttrs.push_back("marker-names");
            requestAttrs.push_back("marker-levels");
            requestAttrs.push_back("marker-high-levels");
            map<string, string> attrs = c.getPrinterAttributes(node.strName.toStdString().c_str(),
                                                               nullptr, &requestAttrs);

            for (auto iter = attrs.begin(); iter != attrs.end(); iter++) {
                if (iter->first == "marker-names") {
                    strMarkerName = QString::fromStdString(iter->second);
                    strNameList = strMarkerName.split('\0');
                } else {
                    strMarkerLevel = QString::fromStdString(iter->second);
                    strLevelList = strMarkerLevel.split('\0');
                }
            }

            for (int i = 0; i < strNameList.size(); i++) {
                QString strName = strNameList.at(i).trimmed();
                QString strLevel = strLevelList.at(i).trimmed();

                if (strName.isEmpty() && strLevel.isEmpty()) {
                    continue;
                }

                SUPPLYSDATA info;
                strName = strName.remove(0, 2);
                strLevel = strLevel.remove(0, 2);
                strcpy(info.name, strName.toStdString().c_str());
                info.level = strLevel.toInt();
                vecMarkInfo.push_back(info);
            }
        } catch (const std::runtime_error &e) {
            qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
        }
    }

    m_mapSupplyInfo.insert(node.strName, vecMarkInfo);
    return (vecMarkInfo.size() > 0);
}

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
    initColorTable();
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
    QVector<SUPPLYSDATA> vecMarkInfo;

    if (node.strUrl.startsWith("socket://")) {
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
            QStringList strLevelList;
            QStringList strNameList;
            QStringList strHighLevels;
            QStringList strTypes;
            QStringList strColors;
            c.init(node.strLoc.toStdString().c_str(), ippPort(), 0);
            vector<string> requestAttrs;
            requestAttrs.push_back("marker-names");
            requestAttrs.push_back("marker-levels");
            requestAttrs.push_back("marker-high-levels");
            requestAttrs.push_back("marker-types");
            requestAttrs.push_back("marker-colors");
            map<string, string> attrs = c.getPrinterAttributes(node.strName.toStdString().c_str(),
                                                               nullptr, &requestAttrs);

            for (auto iter = attrs.begin(); iter != attrs.end(); iter++) {
                if (iter->first == "marker-names") {
                    QString strVal = QString::fromStdString(iter->second);
                    strNameList = strVal.split('\0');
                } else if (iter->first == "marker-levels") {
                    QString strVal = QString::fromStdString(iter->second);
                    strLevelList = strVal.split('\0');
                } else if (iter->first == "marker-high-levels") {
                    QString strVal = QString::fromStdString(iter->second);
                    strHighLevels = strVal.split('\0');
                } else if (iter->first == "marker-types") {
                    QString strVal = QString::fromStdString(iter->second);
                    strTypes = strVal.split('\0');
                } else if (iter->first == "marker-colors") {
                    QString strVal = QString::fromStdString(iter->second);
                    strColors = strVal.split('\0');
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

                if (strHighLevels.isEmpty()) {
                    info.max_capacity = 100;
                } else {
                    QString strMaxCapacity = strHighLevels.at(i).trimmed();

                    if (strMaxCapacity.isEmpty()) {
                        info.max_capacity = 100;
                    } else {
                        strMaxCapacity = strMaxCapacity.remove(0, 2);
                        info.max_capacity = strMaxCapacity.toInt();
                    }
                }

                QString strType = strTypes.at(i).trimmed();
                strType = strType.remove(0, 2);

                if (strType == "toner") {
                    info.type = 3;
                    QString strColor = strColors.at(i).trimmed();
                    strColor = strColor.remove(0, 2);
                    QString strColorName = getColorName(strColor);
                    strcpy(info.color, strColor.toStdString().c_str());
                    strcpy(info.colorName, strColorName.toStdString().c_str());
                } else if (strType == "waste-toner") {
                    info.type = 4;
                }

                vecMarkInfo.push_back(info);
            }
        } catch (const std::runtime_error &e) {
            qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
        }
    }

    m_mapSupplyInfo.insert(node.strName, vecMarkInfo);
    return (vecMarkInfo.size() > 0);
}

void RefreshSnmpBackendTask::initColorTable()
{
    m_colorTable.insert("#000000", "Black");
    m_colorTable.insert("#0000FF", "Blue");
    m_colorTable.insert("#A52A2A", "Brown");
    m_colorTable.insert("#00FFFF", "Cyan");
    m_colorTable.insert("#404040", "Dark-gray");
    m_colorTable.insert("#404040", "Dark gray");
    m_colorTable.insert("#FFCC00", "Dark-yellow");
    m_colorTable.insert("#FFCC00", "Dark yellow");
    m_colorTable.insert("#FFD700", "Gold");
    m_colorTable.insert("#808080", "Gray");
    m_colorTable.insert("#00FF00", "Green");
    m_colorTable.insert("#606060", "Light-black");
    m_colorTable.insert("#606060", "Light black");
    m_colorTable.insert("#E0FFFF", "Light-cyan");
    m_colorTable.insert("#E0FFFF", "Light cyan");
    m_colorTable.insert("#D3D3D3", "Light-gray");
    m_colorTable.insert("#D3D3D3", "Light gray");
    m_colorTable.insert("#FF77FF", "Light-magenta");
    m_colorTable.insert("#FF77FF", "Light magenta");
    m_colorTable.insert("#FF00FF", "Magenta");
    m_colorTable.insert("#FFA500", "Orange");
    m_colorTable.insert("#FF0000", "Red");
    m_colorTable.insert("#C0C0C0", "Silver");
    m_colorTable.insert("#FFFFFF", "White");
    m_colorTable.insert("#FFFF00", "Yellow");
}

QString RefreshSnmpBackendTask::getColorName(const QString &strColor)
{
    QString strTmp = strColor.toUpper();
    return m_colorTable.value(strTmp);
}

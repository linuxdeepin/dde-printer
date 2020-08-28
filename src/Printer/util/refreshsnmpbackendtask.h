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
#ifndef REFRESHSNMPBACKENDTASK_H
#define REFRESHSNMPBACKENDTASK_H

#include "dprinter.h"

#include <cupssnmp.h>
#include <QVector>
#include <QMap>
#include <QThread>

typedef struct SnmpFreshNode
{
    QString strName;
    QString strUrl;
    QString strPpdName;
    QString strLoc;
} SNMPFRESHNODE;

class RefreshSnmpBackendTask : public QThread
{
    Q_OBJECT
public:
    explicit RefreshSnmpBackendTask(QObject *parent = nullptr);
    ~RefreshSnmpBackendTask();

public:
    void setPrinters(const QStringList&);
    void beginTask();
    void stopTask();
    bool isTaskRunning();

public:
    QVector<SUPPLYSDATA> getSupplyData(const QString& strName);

signals:
    void refreshsnmpfinished(const QString&, bool iResult);

public slots:

protected:
    virtual void run();

private:
    bool canGetSupplyMsg(const SNMPFRESHNODE&);
    void initColorTable();
    QString getColorName(const QString&);

private:
    QStringList m_strPrinterNames;
    QMap<QString, QVector<SUPPLYSDATA>> m_mapSupplyInfo;
    QVector<SNMPFRESHNODE> m_vecFreshNode;
    QMap<QString,QString> m_colorTable;
    bool m_bExit;
};

#endif // REFRESHSNMPBACKENDTASK_H

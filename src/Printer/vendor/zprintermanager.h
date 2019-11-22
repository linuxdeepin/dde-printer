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

#ifndef DPRINTERMANAGER_H
#define DPRINTERMANAGER_H

#include "zdevicemanager.h"
#include "cupsconnection.h"

#include <QString>
#include <QMap>
#include <QObject>
#include <QList>
#include <QVariant>

#include <cups/cups.h>

//获取系统已经添加的打印机信息
int GetSystemPrinters();

//尝试通过uri获取打印机信息
int GetPrinterInfo(TDeviceInfo& printer);

//添加系统打印机
int AddSystemPrinter(TDeviceInfo printer, QVariantMap solution);

//移除系统打印机
int RemoveSystemPrinter(QString printername);

//复制系统打印机
int DuplicateSystemPrinter(TDeviceInfo printer, QString newname);

//重命名系统打印机
int RenameSystemPrinter(TDeviceInfo printer, QString newname);

//设置系统打印机信息
int SetSytemPrinterInfo(TDeviceInfo printer);

//设置系统默认打印机
int SetDefaultPrinter(QString printername);
const char* GetDefaultPrinter(void);

//设置打印机是否共享
int SetPrinterShared(QString printername, bool bShared = true);

//设置打印机释放接受任务
int setPrinterAcceptingJobs(const char *name, bool bAccept = true);

//停用/启用打印机
int setPrinterDisable(const char *name, bool bDisable = true);

class DPrinterManager : public QObject
{
    Q_OBJECT

public:
    static DPrinterManager *getInstance();
    ~DPrinterManager();

public:
    bool InitConnection(const char *host_uri, int port, int encryption);
    Connection *getConnection();

private:
    DPrinterManager();
    Q_DISABLE_COPY(DPrinterManager)

private:
    Connection *m_conn;
};

#define g_cupsConnection DPrinterManager::getInstance()->getConnection()

#endif // DPRINTERMANAGER_H

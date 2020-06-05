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

#ifndef ZTASKINTERFACE_H
#define ZTASKINTERFACE_H

#include <QThread>
#include <QString>
#include <QDebug>
#include <QList>

typedef enum enumTaskStat {
    TStat_None = 0,
    TStat_Running,
    TStat_Update,
    TStat_Suc,
    TStat_Fail
} ETaskStatus;

typedef enum enumTaskType {
    TASK_NULL = 0,
    TASK_RefreshKnownDev, /*刷新自动发现的设备列表*/
    TASK_RefreshNetDev, /*刷新网络打印机列表*/
    TASK_InitPPD, /*初始化本地PPD列表信息*/
    TASK_FindLocalDriver, /*查找本地驱动*/
    TASK_FindNetDriver, /*从服务器查找驱动*/
    TASK_TryPrinterInfo, /*尝试获取打印机信息,判断是否需要驱动*/
    TASK_GetInstalledPrinters, /*获取已经添加的打印机信息*/
    TASK_AddPrinter, /*添加打印机*/
    TASK_PrintTestPage, /*打印测试页*/
    TASK_TroubleShoot, //故障排查
    Task_CupsMonitor, //cups状态监视器
    TASK_MAX
} ETaskType;

class TaskInterface : public QThread
{
    Q_OBJECT

public:
    TaskInterface(int id, QObject *parent = nullptr);
    virtual ~TaskInterface();

    virtual void stop();

    QString getErrorString();

    int getErrCode();

protected:
    //返回0表示成功
    virtual int doWork() = 0;
    void run();

    int m_iTaskId;
    QString m_strLastErr;
    int m_errCode;
    bool m_bQuit;

signals:
    void signalStatus(int, int);
    void signalUpdateProgress(int, QString);
};

#endif // ZTASKINTERFACE_H

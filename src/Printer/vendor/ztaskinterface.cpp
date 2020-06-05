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

#include "ztaskinterface.h"

#include <QEventLoop>
#include <QTimer>

static const char *g_taskText[] = {"null",
                                   "Refresh devices by backends",
                                   "Refresh devices by host",
                                   "Init ppds",
                                   "not use",
                                   "not use",
                                   "not use",
                                   "not use",
                                   "not use",
                                   "not use",
                                   "Toruble shoot",
                                   "Cups monitor",
                                   "max value"};

TaskInterface::TaskInterface(int id, QObject *parent)
    : QThread(parent)
{
    m_bQuit = false;
    m_iTaskId = id;
    m_errCode = 0;
}

TaskInterface::~TaskInterface()
{
    qInfo() << g_taskText[m_iTaskId];
    stop();
}

void TaskInterface::stop()
{
    m_bQuit = true;

    qInfo() << "Stop task " << g_taskText[m_iTaskId];
    this->disconnect();
    if (this->isRunning()) {
        this->quit();
        this->wait();
    }
}

QString TaskInterface::getErrorString()
{
    return m_strLastErr;
}

int TaskInterface::getErrCode()
{
    return m_errCode;
}

void TaskInterface::run()
{
    qInfo() << "Task " << g_taskText[m_iTaskId] << " running...";
    int iRet = 0;
    emit signalStatus(m_iTaskId, TStat_Running);
    iRet = doWork();

    if (m_bQuit)
        return;

    //如果doWork中没有设置ErrCode，返回值作为errCode
    if (0 == getErrCode())
        m_errCode = iRet;

    qInfo() << "Task " << g_taskText[m_iTaskId] << " finished " << iRet;
    if (0 == iRet)
        emit signalStatus(m_iTaskId, TStat_Suc);
    else
        emit signalStatus(m_iTaskId, TStat_Fail);
}

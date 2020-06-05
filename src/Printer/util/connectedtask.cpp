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
#include "connectedtask.h"

ConnectedTask::ConnectedTask(const QString &name, QObject *parent)
    : QThread(parent)
    , m_printerName(name)
{
    m_pCheckCon = nullptr;
}

ConnectedTask::~ConnectedTask()
{
}

void ConnectedTask::run()
{
    m_pCheckCon = new CheckConnected(m_printerName);
    bool passed = m_pCheckCon->isPass();
    delete m_pCheckCon;
    //用printerName区分信号的来源
    emit signalResult(passed, m_printerName);
}

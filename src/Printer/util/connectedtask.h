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
#ifndef CONNECTEDTASK_H
#define CONNECTEDTASK_H

#include "ztroubleshoot_p.h"

#include <QThread>

class ConnectedTask : public QThread
{
    Q_OBJECT
public:
    explicit ConnectedTask(const QString &name, QObject *parent = nullptr);
    virtual ~ConnectedTask() override;

    void setSuspended(bool suspended);

protected:
    void run() override;

private:
    CheckConnected *m_pCheckCon;
    QString m_printerName;

signals:
    void signalResult(bool connected, const QString &printerName);
};
#endif // CONNECTEDTASK_H

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

#ifndef ZTROUBLESHOOT_P_H
#define ZTROUBLESHOOT_P_H

#include "ztroubleshoot.h"

class CheckCupsServer : public TroubleShootJob
{
    Q_OBJECT

public:
    CheckCupsServer(QObject *parent=nullptr);

    bool isPass() Q_DECL_OVERRIDE;
    QString getJobName() Q_DECL_OVERRIDE;
};

class CheckDriver : public TroubleShootJob
{
    Q_OBJECT

public:
    CheckDriver(const QString &printerName, QObject *parent=nullptr);

    bool isPass() Q_DECL_OVERRIDE;
    QString getJobName() Q_DECL_OVERRIDE;
};

class CheckConnected : public TroubleShootJob
{
    Q_OBJECT

public:
    CheckConnected(const QString &printerName, QObject *parent=nullptr);

    bool isPass() Q_DECL_OVERRIDE;
    QString getJobName() Q_DECL_OVERRIDE;
};

class CheckAttributes : public TroubleShootJob
{
    Q_OBJECT

public:
    CheckAttributes(const QString &printerName, QObject *parent=nullptr);

    bool isPass() Q_DECL_OVERRIDE;
    QString getJobName() Q_DECL_OVERRIDE;
};

#endif//ZTROUBLESHOOT_P_H

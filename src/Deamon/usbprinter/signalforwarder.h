/*
 * Copyright (C) 2019 ~ 2020 Uniontech Software Co., Ltd.
 *
 * Author:     liurui <liurui@uniontech.com>
 *
 * Maintainer: liurui <liurui@uniontech.com>
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
#ifndef SIGNALFORWARDER_H
#define SIGNALFORWARDER_H

#include <QObject>

class SignalForwarder : public QObject
{
    Q_OBJECT
public:
    explicit SignalForwarder(QObject *parent = nullptr);

signals:
    void deviceStatusChanged(const QString &defaultPrinterName, int status);
public slots:
    void slotDeviceStatusChanged(const QString &defaultPrinterName, int status);
};

#endif // SIGNALFORWARDER_H

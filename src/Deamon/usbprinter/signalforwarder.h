// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

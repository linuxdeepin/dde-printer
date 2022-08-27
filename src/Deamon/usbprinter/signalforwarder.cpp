// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "signalforwarder.h"

SignalForwarder::SignalForwarder(QObject *parent) : QObject(parent)
{

}

void SignalForwarder::slotDeviceStatusChanged(const QString &defaultPrinterName, int status)
{
    emit deviceStatusChanged(defaultPrinterName, status);
}

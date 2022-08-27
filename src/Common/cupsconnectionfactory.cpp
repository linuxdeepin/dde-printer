// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cupsconnectionfactory.h"
#include "zsettings.h"

#include <QDebug>

std::unique_ptr<Connection> CupsConnectionFactory::createConnection(QString strHost, int port, int encryption)

{
    std::unique_ptr<Connection> connectionPtr = std::unique_ptr<Connection>(new Connection());
    try {
        if (0 != connectionPtr->init(strHost.toUtf8().data(), port, encryption)) {
            qWarning() << "Unable to connect cups server"  ;
            connectionPtr.reset();
        }
    } catch (const std::exception &ex) {
        qWarning() << "Got execpt: " << QString::fromUtf8(ex.what());
        connectionPtr.reset();

    }
    return connectionPtr;
}

std::unique_ptr<Connection> CupsConnectionFactory::createConnectionBySettings()
{
    std::unique_ptr<Connection> connectionPtr = std::unique_ptr<Connection>(new Connection());
    try {
        if (0 != connectionPtr->init(g_Settings->getCupsServerHost().toLocal8Bit(), g_Settings->getCupsServerPort(), g_Settings->getCupsServerEncryption())) {
            qWarning() << "Unable to connect deafult cups server";
            connectionPtr.reset();
        }
    } catch (const std::exception &ex) {
        qWarning() << "Got execpt: " << QString::fromUtf8(ex.what());
        connectionPtr.reset();
    }
    return connectionPtr;
}

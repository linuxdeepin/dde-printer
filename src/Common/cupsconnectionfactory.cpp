/*
 * Copyright (C) 2019 ~ 2019 Uniontech Software Co., Ltd.
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
#include "cupsconnectionfactory.h"
#include "zsettings.h"

#include <QDebug>

std::unique_ptr<Connection> CupsConnectionFactory::createConnection(QString strHost, int port, int encryption)

{
    std::unique_ptr<Connection> connectionPtr = std::unique_ptr<Connection>(new Connection());
    try {
        if (0 != connectionPtr->init(strHost.toUtf8().data(), port, encryption)) {
            qWarning() << "Unable to connect " << strHost;
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
            qWarning() << "Unable to connect " << g_Settings->getCupsServerHost().toLocal8Bit();
            connectionPtr.reset();
        }
    } catch (const std::exception &ex) {
        qWarning() << "Got execpt: " << QString::fromUtf8(ex.what());
        connectionPtr.reset();
    }
    return connectionPtr;
}

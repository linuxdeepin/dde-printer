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
#ifndef CUPSCONNECTIONFACTORY_H
#define CUPSCONNECTIONFACTORY_H

#include "cupsconnection.h"

#include <QString>

#include <memory>
using namespace std;
class CupsConnectionFactory
{
public:

    static std::unique_ptr<Connection> createConnection(QString strHost, int port, int encryption = 0);
    static std::unique_ptr<Connection> createConnectionBySettings();
private:
    CupsConnectionFactory(){}
    Q_DISABLE_COPY(CupsConnectionFactory)
};


#define g_cupsConnection CupsConnectionFactory::createConnectionBySettings()
#endif // CUPSCONNECTIONFACTORY_H

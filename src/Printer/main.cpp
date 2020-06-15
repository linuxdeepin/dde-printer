/*
 * Copyright (C) 2019 ~ 2020 Uniontech Software Co., Ltd.
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

#include "printerapplication.h"

#include <DApplication>
#include <DLog>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    int iRet = 0;

    DApplication::loadDXcbPlugin();
    DApplication a(argc, argv);

    if (0 != g_printerApplication->create()) {
        qCritical() << "Create printer application failed";
        return -1;
    }

    if (0 != g_printerApplication->launchWithMode(a.arguments())) {
        qCritical() << "Init printer application failed";
        return -2;
    }

    iRet = a.exec();
    g_printerApplication->stop();

    return iRet;
}

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
#include "reviselogger.h"

#include <DApplication>
#include <DLog>

#include <DApplicationSettings>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{

#if (DTK_VERSION >= DTK_VERSION_CHECK(5, 6, 8, 7))
    DLogManager::registerLoggingRulesWatcher("dde-printer");
#endif

    MLogger loggerConf;

    int iRet = 0;
    /*需要在构造app之前设置这个属性,自适应屏幕缩放*/
    DApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    DApplication a(argc, argv);

    if (0 != g_printerApplication->create()) {
        qCritical() << "Create printer application failed";
        return -1;
    }

    if (0 != g_printerApplication->launchWithMode(a.arguments())) {
        qCritical() << "Init printer application failed";
        return -2;
    }
    /*自动保存主题设置,需要在main里面设置*/
    DApplicationSettings saveTheme;

    iRet = a.exec();
    g_printerApplication->stop();

    return iRet;
}

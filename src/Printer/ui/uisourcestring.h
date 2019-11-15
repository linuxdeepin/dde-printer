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
#ifndef UISOURCESTRING_H
#define UISOURCESTRING_H
#include <QObject>
#define UI_PRINTERSEARCH_MANUAL QObject::tr("Select a driver")
#define UI_PRINTERSEARCH_URITIP    "ipp://printer.mydomain/ipp\n" \
    "ipp://cups-sever/printers/printer-queue\n" \
    "smb://workgroup/username:password@hostname/printer_name\n" \
    "lpd://sever/printer-queue\n" \
    "socket://server[:port]"
#define UI_PRINTERSEARCH_DRIVER QObject::tr("Driver")
#define UI_PRINTERSEARCH_INSTALLDRIVER_NEXT QObject::tr("Next")
#define UI_PRINTERSEARCH_INSTALLDRIVER_AUTO QObject::tr("Start the installation")

#define UI_PRINTERDRIVER_PPDLABEL_NORMAL QObject::tr("Drag a PPD file here \n or")
#define UI_PRINTERDRIVER_PPDBTN_NORMAL QObject::tr("Select a PPD file")

#define UI_PRINTERSHOW_TROUBLE QObject::tr("Troubleshoot")
#define UI_PRINTERSHOW_CANCEL QObject::tr("Cancel")
#endif // UISOURCESTRING_H

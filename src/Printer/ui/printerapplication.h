/*
 * Copyright (C) 2019 ~ 2020 Uniontech Software Co., Ltd.
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

#ifndef PRINTERAPPLICATION_H
#define PRINTERAPPLICATION_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QDBusMessage>

class JobManagerWindow;
class DPrintersShowWindow;

class PrinterApplication : public QObject
{
    Q_OBJECT

public:
    static PrinterApplication *getInstance();

    int create();
    int launchWithMode(const QStringList &arguments);
    int stop();

    int showMainWindow();
    int showJobsWindow();

public slots:
    void slotMainWindowClosed();

protected slots:
    void slotNewProcessInstance(qint64 pid, const QStringList &arguments);

protected:
    PrinterApplication();
    ~PrinterApplication();

private:
    JobManagerWindow *m_jobsWindow;
    DPrintersShowWindow *m_mainWindow;
    QSystemTrayIcon *m_systemTray;
};

#define g_printerApplication PrinterApplication::getInstance()

#endif //PRINTERAPPLICATION_H

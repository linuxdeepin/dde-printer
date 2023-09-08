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
#ifndef HELPERINTERFACE_H
#define HELPERINTERFACE_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QTimer>

#include "zcupsmonitor.h"

class HelperInterface : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", SERVICE_INTERFACE_NAME)
public:
    explicit HelperInterface(CupsMonitor *pCupsMonitor, QObject *parent = nullptr);
    ~HelperInterface();

    void registerDBus();
    void unRegisterDBus();
signals:
    void signalJobStateChanged(int id, int state, const QString &message);
    void signalPrinterStateChanged(const QString &printer, int state, const QString &message);
    void signalPrinterDelete(const QString &printer);
    void signalPrinterAdd(const QString &printer);
    void timeoutExit();
    void deviceStatusChanged(const QString &defaultPrinterName, int status);

public slots:
    //dbus接口
    bool isJobPurged(int id);
    QString getJobNotify(const QMap<QString, QVariant> &job);
    QString getStateString(int iState);
    void setDdePrinterState();

protected:
    void slotShowTrayIcon(bool bShow);
    void showJobsWindow();

private:
    CupsMonitor *m_pCupsMonitor;
    QSystemTrayIcon *m_pSystemTray;
    QTimer* m_timer;
};

#endif // HELPERINTERFACE_H

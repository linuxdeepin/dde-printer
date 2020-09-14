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

#ifndef ZCUPSMONITOR_H
#define ZCUPSMONITOR_H

#include "ztaskinterface.h"
#include "config.h"

#include <QMutex>
#include <QSet>
#include <QDBusMessage>
#include <QTime>
#include <QSystemTrayIcon>


class CupsMonitor : public QThread
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", SERVICE_INTERFACE_NAME)
public:
    CupsMonitor(QObject *parent = nullptr);
    ~CupsMonitor() override;

    void initTranslations();
    int initSubscription();
    bool initWatcher();
    void clearSubscriptions();
    void stop();
    QString getJobMessage(int id);
    int getPrinterState(const QString &printer);

    void registerDBus();
    void unRegisterDBus();
public slots:
    //dbus接口
    bool isJobPurged(int id);
    QString getJobNotify(const QMap<QString, QVariant> &job);
    QString getStateString(int iState);

protected:

    void run() override;
    int doWork();

    bool insertJobMessage(int id, int state, const QString &message);

    int cancelSubscription();
    int createSubscription();
    int getNotifications(int &notifysSize);
    int resetSubscription();

    // expired: timeout in millisecond
    //           0 - never expired
    //          -1 - server dependent
    int sendDesktopNotification(int replaceId, const QString &summary, const QString &body, int expired);

    bool isCompletedState(int state);
    void slotShowTrayIcon(bool bShow);
    void showJobsWindow();


protected slots:
    void notificationInvoke(unsigned int, QString);
    void notificationClosed(unsigned int, unsigned int);
    void spoolerEvent(QDBusMessage);

signals:
    void signalJobStateChanged(int id, int state, const QString &message);
    void signalPrinterStateChanged(const QString &printer, int state, const QString &message);
    void signalPrinterDelete(const QString &printer);
    void signalPrinterAdd(const QString &printer);

private:
    QMap<int, QString> m_jobMessages;
    QMap<QString, int> m_printersState;
    QStringList m_stateStrings;
    QMutex m_mutex;

    int m_subId;
    int m_seqNumber;
    int m_jobId;

    bool m_bQuit;

    QSet<unsigned int> m_pendingNotification;
    QMap<int, QTime> m_processingJob;

    QSystemTrayIcon *m_systemTray;
};


#endif //ZCUPSMONITOR_H

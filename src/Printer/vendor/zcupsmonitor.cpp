/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
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

#include "zcupsmonitor.h"
#include "common.h"
#include "cupsconnection.h"
#include "config.h"
#include "qtconvert.h"
#include "zsettings.h"
#include "cupsattrnames.h"
#include "printerapplication.h"
#include "zjobmanager.h"
#include "dprintermanager.h"

#include <DApplication>

#include <QMap>
#include <QVariant>
#include <QDBusPendingReply>
#include <QStringList>
#include <DNotifySender>
#include <QDBusConnection>

DWIDGET_USE_NAMESPACE

#define SUB_URI "/"
#define IDLEEXIT 1000 * 60 * 5
#define PROCESSINGTIP 1000 * 60

DCORE_USE_NAMESPACE

CupsMonitor* CupsMonitor::getInstance()
{
    static CupsMonitor *instance = nullptr;
    if (nullptr == instance) {
        instance = new CupsMonitor();
    }

    return instance;
}

CupsMonitor::CupsMonitor(QObject *parent)
    : TaskInterface(Task_CupsMonitor, parent),
    m_jobId(0)
{
    m_subId = -1;
    m_seqNumber = -1;
}

void CupsMonitor::initTranslations()
{
    if (m_stateStrings.isEmpty()) {
        m_stateStrings.append("");
        m_stateStrings.append("");
        m_stateStrings.append("");
        m_stateStrings.append(tr("Queuing"));
        m_stateStrings.append(tr("Paused"));
        m_stateStrings.append(tr("Printing"));
        m_stateStrings.append(tr("Stopped"));
        m_stateStrings.append(tr("Canceled"));
        m_stateStrings.append(tr("Error"));
        m_stateStrings.append(tr("Completed"));
    }
}

QString CupsMonitor::getStateString(int iState)
{
    return iState < m_stateStrings.count() ? m_stateStrings[iState] : QString();
}

bool CupsMonitor::insertJobMessage(int id, int state, const QString &message)
{
    QString str;
    int times = 0;
    bool hasRuningJobs = false;

    {
        QMutexLocker locker(&m_mutex);

        //获取对应任务状态更新的次数，如果大于5次重置保存的次数，通知界面刷新一次
        str = m_jobMessages.value(id);
        if (!str.isEmpty()) {
            str = str.split(" ").first();

            if (str.startsWith(QString::number(state))) {
                times = str.mid(1).toInt();
                times = times > 5 ? 0 : times;
            }
        }
        times++;

        str = QString::number(state)+QString::number(times)+" "+message;
        m_jobMessages.insert(id, str);

        QStringList msglist = m_jobMessages.values();
        foreach (str, msglist) {
            if (!str.isEmpty()) {
                int iState = str.left(1).toInt();

                if (!g_jobManager->isCompletedState(iState)) {
                    hasRuningJobs = true;
                    break;
                }
            }
        }
    }

    emit signalShowTrayIcon(hasRuningJobs);

    //只有处理中的状态才通过事件触发的次数过滤事件
    if (IPP_JSTATE_PROCESSING != state) {
        times = 1;
    }

    return times == 1;
}

bool CupsMonitor::isJobPurged(int id)
{
    QString str = getJobMessage(id);

    return str.indexOf("Job purged") >= 0;
}

QString CupsMonitor::getJobMessage(int id)
{
    QString str;
    QStringList list;

    {
        QMutexLocker locker(&m_mutex);
        str = m_jobMessages.value(id);
    }

    if (!str.isEmpty()) {
        list = str.split(" ");
        list.removeFirst();
        str = list.join(" ");
    }

    return str;
}

QString CupsMonitor::getJobNotify(const QMap<QString, QVariant> &job)
{
    int iState = job[JOB_ATTR_STATE].toString().toInt();
    QString strState = m_stateStrings[iState];
    int id = job[JOB_ATTR_ID].toInt();

    if (IPP_JSTATE_ABORTED == iState || IPP_JSTATE_PROCESSING == iState || IPP_JSTATE_STOPPED == iState) {
        QString jobmessage = getJobMessage(id);
        QString printermessage = job[JOB_ATTR_STATE_MEG].toString();
        if (!jobmessage.isEmpty()) {
            strState += QString(" [%1]").arg(jobmessage);
        } else if (!printermessage.isEmpty()) {
            strState += QString(" [%1]").arg(printermessage);
        }
    }

    return strState;
}

void CupsMonitor::clearSubscriptions()
{
    try {
        vector<map<string, string>> subs = g_cupsConnection->getSubscriptions(SUB_URI, true, -1);
        for (size_t i=0;i<subs.size();i++) {
            dumpStdMapValue(subs[i]);
            m_subId = attrValueToQString(subs[i]["notify-subscription-id"]).toInt();
            g_cupsConnection->cancelSubscription(m_subId);
        }
    }catch(const std::exception &ex) {
        qWarning() << "Got execpt: " << QString::fromUtf8(ex.what());
        return ;
    }
}

int CupsMonitor::createSubscription()
{
    m_subId = g_Settings->getSubscriptionId();

    try {
        if (-1 != m_subId) {
            bool isExists = false;
            vector<map<string, string>> subs = g_cupsConnection->getSubscriptions(SUB_URI, true, -1);
            for (size_t i=0;i<subs.size();i++) {
                dumpStdMapValue(subs[i]);

                if (m_subId == attrValueToQString(subs[i]["notify-subscription-id"]).toInt()) {
                    qInfo() << "Use last subscription id: " << m_subId;
                    isExists = true;
                    break;
                }
            }

            if (!isExists) m_subId = -1;
        }
    }catch(const std::exception &ex) {
        qWarning() << "Got execpt: " << QString::fromUtf8(ex.what());
        m_subId = -1;
    }

    try{
        if (-1 == m_subId) {
            vector<string> events = {"printer-deleted", "printer-state-changed", "job-progress", "job-state-changed"};
            m_subId = g_cupsConnection->createSubscription(SUB_URI, &events, 0, nullptr, 86400, 0, nullptr);
            g_Settings->setSubscriptionId(m_subId);
            g_Settings->setSequenceNumber(0);
            qDebug() << "createSubscription id: " << m_subId;
        }
    }catch(const std::exception &ex) {
        qWarning() << "Got execpt: " << QString::fromUtf8(ex.what());
        m_subId = -1;
    }

    return m_subId;
}

int CupsMonitor::getNotifications(int& notifysSize)
{
    if (-1 == m_subId) return -1;

    try {
        bool skip = m_jobId > 0;

        vector<map<string, string>> notifys = g_cupsConnection->getNotifications(m_subId, m_seqNumber, nullptr, nullptr);
        notifysSize = notifys.size();

        if (notifysSize)
            qDebug() << "Got number:" << notifysSize << "after sequence:" << m_seqNumber;

        for(int i=0;i<notifysSize;i++) {
            map<string, string> info = notifys.at(i);
            int number = attrValueToQString(info[CUPS_NOTIY_SEQ_NUM]).toInt();
            QString strevent = attrValueToQString(info[CUPS_NOTIY_EVENT]);

            qDebug() << "******************************************************";

            dumpStdMapValue(info);

            if (number >= m_seqNumber) {
                m_seqNumber = number+1;
                g_Settings->setSequenceNumber(m_seqNumber);
            }


            if (strevent.startsWith("job-")) {
                int iState = attrValueToQString(info[JOB_ATTR_STATE]).toInt();
                int iJob= attrValueToQString(info[CUPS_NOTIY_JOBID]).toInt();
                QString strReason = attrValueToQString(info[CUPS_NOTIY_TEXT]);
                QStringList list = attrValueToQString(info[JOB_ATTR_NAME]).split("/", QString::SkipEmptyParts);
                QString strJobName = list.isEmpty()?"":list.last();
                qInfo() << "Got a job event: " << iJob << iState << strReason;

                if (iJob == m_jobId) {
                    skip = false;
                    m_jobId = 0;
                }

                if (skip) continue;

                //通过判断同一个id，同一个状态插入的次数判断是否触发信号
                if (insertJobMessage(iJob, iState, strReason)) {
                    qInfo() << "Emit job state changed signal" << iJob << iState << strReason;
                    emit signalJobStateChanged(iJob, iState, strReason);
                }

                switch (iState) {
                    case IPP_JSTATE_PROCESSING:
                        if (m_processingJob.contains(iJob)) {
                            const QTime& t = m_processingJob[iJob];
                            if (!t.isNull() && t.elapsed() > PROCESSINGTIP) {
                                strReason = tr("%1 timed out, reason: %2").arg(strJobName).arg(strReason);
                                sendDesktopNotification(0, qApp->productName(), strReason, 3000);
                                m_processingJob[iJob] = QTime();
                            }
                        } else {
                            QTime t;
                            t.start();
                            m_processingJob.insert(iJob, t);
                        }
                        break;
                    case IPP_JSTATE_COMPLETED:
                    case IPP_JSTATE_STOPPED:
                    case IPP_JSTATE_ABORTED:
                    {
                        if (IPP_JSTATE_COMPLETED == iState) {
                            strReason = tr("%1 printed successfully, please take away the paper in time!").arg(strJobName);
                        } else {
                            strReason = tr("%1 %2, reason: %3")
                                    .arg(strJobName).arg(getStateString(iState).toLower()).arg(strReason);
                        }
                        sendDesktopNotification(0, qApp->productName(), strReason, 3000);

                        Q_FALLTHROUGH();
                    }
                    case IPP_JOB_CANCELLED:
                        m_processingJob.remove(iJob);
                        break;
                    default:
                        break;
                }
            } else {
                if (skip) continue;

                if ("printer-state-changed" == strevent) {
                    int iState = attrValueToQString(info[CUPS_OP_STATE]).toInt();
                    QString printerName = attrValueToQString(info[CUPS_OP_NAME]);
                    QString strReason = attrValueToQString(info[CUPS_OP_STATIE_RES]);

                    if ("none" == strReason)
                        strReason = attrValueToQString(info[CUPS_NOTIY_TEXT]);

                    qDebug() << "Printer state changed: " << printerName << iState << strReason;

                    //只有状态改变的时候才触发信号
                    if (m_printersState.value(printerName, -1) != iState) {
                        qInfo() << "Emit printer state changed signal: " << printerName << iState << strReason;
                        m_printersState.insert(printerName, iState);
                        emit signalPrinterStateChanged(printerName, iState, strReason);
                    }
                } else if ("printer-deleted" == strevent) {
                    QString printerName = attrValueToQString(info[CUPS_OP_NAME]);

                    emit signalPrinterDelete(printerName);
                }
            }
        }

        if (notifysSize)
            qDebug() << "------------------------------------------------------------";
    }catch(const std::exception &ex) {
        qWarning() << "Got execpt: " << QString::fromUtf8(ex.what());
        return -1;
    }

    return 0;
}

int CupsMonitor::cancelSubscription()
{
    try {
        if (-1 != m_subId)
            g_cupsConnection->cancelSubscription(m_subId);
    }catch(const std::exception &ex) {
        qWarning() << "Got execpt: " << QString::fromUtf8(ex.what());
        return -1;
    }

    return 0;
}

int CupsMonitor::initSubscription()
{
    if (-1 != m_subId) return 0;

    // 不要移动 m_subId 和 m_seqNumber 初始化的顺序
    m_subId = createSubscription();
    m_seqNumber = g_Settings->getSequenceNumber();

    qInfo() << QString("subscription: %1, sequence: %2").arg(m_subId).arg(m_seqNumber);
    if (-1 == m_subId) {
        qWarning() << "Invaild subcription id";
        return -1;
    }

    return 0;
}

int CupsMonitor::resetSubscription()
{
    cancelSubscription();
    m_subId = m_seqNumber = -1;

    return initSubscription();
}

int CupsMonitor::doWork()
{
    m_pendingNotification.clear();
    m_processingJob.clear();

    QTime t;
    t.start();

    while (!m_bQuit) {
        int size = 0;
        if (0 != getNotifications(size) && 0 != resetSubscription()) {
            break;
        }

        m_jobId = 0;

        if (size > 0)
            t.restart();

        if (t.elapsed() > IDLEEXIT)
            break;

        sleep(1);
    }

    return 0;
}

void CupsMonitor::stop()
{
    cancelSubscription();

    TaskInterface::stop();
}

bool CupsMonitor::initWatcher()
{
    QDBusConnection conn = QDBusConnection::systemBus();
    bool success = conn.connect("", "/com/redhat/PrinterSpooler", "com.redhat.PrinterSpooler", "", this, SLOT(spoolerEvent(QDBusMessage)));

    //启动前获取未完成的任务列表，防止遗漏没监听到的任务
    map<int, map<string, string>> jobs;
    if (0 == g_jobManager->getJobs(jobs, WHICH_JOB_RUNING, 1)) {
        map<int, map<string, string>>::iterator itJobs;

        if (jobs.size() > 0) {
            for (itJobs=jobs.begin();itJobs!=jobs.end();itJobs++) {
                map<string, string> info = itJobs->second;
                int iState = attrValueToQString(info[JOB_ATTR_STATE]).toInt();
                QString message = attrValueToQString(info[JOB_ATTR_STATE_MEG]);
                insertJobMessage(itJobs->first, iState, message);
            }
            start();
        }
    }

    if (!success) {
        qInfo() << "failed to connect spooler dbus";
    }
    return success;
}

int CupsMonitor::sendDesktopNotification(int replaceId, const QString& summary, const QString& body, int expired)
{
    int ret = 0;

    QDBusPendingReply<unsigned int> reply = DUtil::DNotifySender(summary)
        .appName("dde-printer")
        .appIcon(":/images/printer.svg")
        .appBody(body)
        .replaceId(replaceId)
        .timeOut(expired)
        .actions(QStringList() << "default")
        .call();

    reply.waitForFinished();
    if (!reply.isError())
        ret = reply.value();

    if (ret > 0)
        m_pendingNotification.insert(ret);

    QDBusConnection conn = QDBusConnection::sessionBus();
    conn.connect("", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", "ActionInvoked", this, SLOT(notificationInvoke(unsigned int, QString)));
    conn.connect("", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", "NotificationClosed", this, SLOT(notificationClosed(unsigned int, unsigned int)));

    return ret;
}

void CupsMonitor::notificationInvoke(unsigned int notificationId, QString action)
{
    Q_UNUSED(action);
    if (m_pendingNotification.contains(notificationId))
        g_printerApplication->showJobsWindow();
}

void CupsMonitor::notificationClosed(unsigned int notificationId, unsigned int reason)
{
    Q_UNUSED(reason);
    m_pendingNotification.remove(notificationId);
}

void CupsMonitor::spoolerEvent(QDBusMessage msg)
{
    QList<QVariant> args = msg.arguments();
    qDebug() << args;

    if (args.size() == 3) {
        if (!isRunning())
            m_jobId = args[1].toInt();
        start();
    }
}

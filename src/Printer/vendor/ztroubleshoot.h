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

#ifndef ZTROUBLESHOTT_H
#define ZTROUBLESHOTT_H

#include "ztaskinterface.h"

#include <QObject>
#include <QList>
#include <QEventLoop>

class TroubleShootJob : public QObject
{
    Q_OBJECT

public:
    TroubleShootJob(const QString &printerName, QObject *parent=nullptr)
        :QObject(parent)
    {
        m_printerName = printerName;
        m_bQuit = false;
    }

    virtual bool isPass() = 0;

    virtual QString getJobName() = 0;

    virtual void stop(){m_bQuit = true;}

    QString getMessage(){return m_strMessage;}

signals:
    void signalStateChanged(int state, const QString &message);

protected:
    QString m_printerName;
    QString m_strMessage;
    bool    m_bQuit;
};

class PrinterTestJob : public TroubleShootJob
{
    Q_OBJECT

public:
    PrinterTestJob(const QString &printerName, QObject *parent=nullptr, bool bSync = true);
    ~PrinterTestJob() Q_DECL_OVERRIDE;

    bool isPass() Q_DECL_OVERRIDE;
    QString getJobName() Q_DECL_OVERRIDE;

    void stop() Q_DECL_OVERRIDE;

protected:
    void findRunningJob();

protected slots:
    void slotJobStateChanged(int id, int state, const QString &message);

private:
    QEventLoop  *m_eventLoop;
    int         m_jobId;
    bool        m_bSync;
};

class TroubleShoot : public TaskInterface
{
    Q_OBJECT

public:
    TroubleShoot(const QString &printerName, QObject *parent=nullptr);
    ~TroubleShoot() Q_DECL_OVERRIDE;

    int addJob(TroubleShootJob *job);

    QList<TroubleShootJob*> getJobs();

    void stop() Q_DECL_OVERRIDE;

protected:
    int doWork() Q_DECL_OVERRIDE;

private:
    QString     m_printerName;

    QList<TroubleShootJob*> m_jobs;
};

#endif

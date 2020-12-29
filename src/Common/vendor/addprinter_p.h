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

#ifndef ADDPRINTER_P_H
#define ADDPRINTER_P_H

#include "addprinter.h"
#include "zdevicemanager.h"

#include <QProcess>
#include <QDBusMessage>

class PrinterServerInterface;

typedef struct tagPackageInfo {
    QString toString() { return packageName + ": " + packageVer; }

    QString packageName;
    QString packageVer;
} TPackageInfo;

class InstallInterface : public QObject
{
    Q_OBJECT

public:
    explicit InstallInterface(QObject *parent = nullptr);

    void setPackages(const QList<TPackageInfo> &packages);
    void startInstallPackages();

    QString getErrorString();

    virtual void stop();

signals:
    void signalStatus(int);

protected slots:
    void propertyChanged(const QDBusMessage &msg);

protected:
    QList<TPackageInfo> m_packages;
    QStringList m_installPackages;
    bool m_bQuit;
    QString m_jobPath;
    QString m_strType;
    QString m_strStatus;
    QString m_strErr;
};

class InstallDriver : public InstallInterface
{
    Q_OBJECT

public:
    InstallDriver(const QMap<QString, QVariant> &solution, QObject *parent = nullptr);

    void doWork();

    void stop();

protected slots:
    void slotServerDone(int iCode, const QByteArray &result);

private:
    QMap<QString, QVariant> m_solution;
};

class AddCanonCAPTPrinter : public AddPrinterTask
{
    Q_OBJECT

public:
    AddCanonCAPTPrinter(const TDeviceInfo &printer, const QMap<QString, QVariant> &solution, const QString &uri, QObject *parent = nullptr);

    void stop();

protected:
    int addPrinter();

protected slots:
    void slotProcessFinished(int iCode, QProcess::ExitStatus exitStatus);

private:
    QProcess m_proc;
};

class DefaultAddPrinter : public AddPrinterTask
{
    Q_OBJECT

public:
    DefaultAddPrinter(const TDeviceInfo &printer, const QMap<QString, QVariant> &solution, const QString &uri, QObject *parent = nullptr);

    void stop();

protected:
    int addPrinter();

protected slots:
    void slotProcessFinished(int iCode, QProcess::ExitStatus exitStatus);

private:
    QProcess m_proc;
};

class FixHplipBackend : public QObject
{
    Q_OBJECT

public:
    FixHplipBackend(QObject *parent = nullptr);

    int startFixed();
    QString getMatchHplipUri(const QString &strUri);

    void stop();

    QString getErrorString();

signals:
    void signalStatus(int);

protected slots:
    void slotDeviceStatus(int id, int status);
    void slotInstallStatus(int status);

private:
    RefreshDevicesByBackendTask *m_deviceTask;
    TBackendSchemes m_hplipBackend;

    InstallInterface *m_installer;

    QString m_strError;
};

#endif //ADDPRINTER_P_H

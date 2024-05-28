// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "service.h"

#include <QCoreApplication>
#include <QTimer>
#include <QDebug>

Service::Service(QObject *parent) : QObject(parent)
{
    m_proc = new QProcess;

    connect(m_proc, &QProcess::stateChanged, [=](QProcess::ProcessState state) {
        if (state == QProcess::ProcessState::NotRunning) {
            QCoreApplication::exit();
        }
    });
}

Service::~Service()
{
    delete m_proc;
    m_proc = nullptr;
}

void Service::LaunchAutoStart(const QString &filePath)
{
    QTimer::singleShot(100, QCoreApplication::instance(), &QCoreApplication::quit);
    QString scriptPath = filePath + "autostart.sh";
    QProcess::startDetached(scriptPath,  QStringList() << "-c");
    return;
}

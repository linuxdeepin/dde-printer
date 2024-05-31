// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QProcess>

#define SERVICE_MANAGER_INTERFACE_NAME "com.deepin.printer.manager"
#define SERVICE_MANAGER_INTERFACE_PATH "/com/deepin/printer/manager"

class Service : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", SERVICE_MANAGER_INTERFACE_NAME)
public:
    explicit Service(QObject *parent = Q_NULLPTR);
    ~Service();

signals:

public slots:
    void LaunchAutoStart(const QString &filePath);

private:
    QProcess *m_proc;
};

// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZDRIVERMANAGER_P_H
#define ZDRIVERMANAGER_P_H

#include "ztaskinterface.h"

class RefreshLocalPPDS : public TaskInterface
{
    Q_OBJECT

protected:
    explicit RefreshLocalPPDS(QObject *parent = nullptr)
        : TaskInterface(TASK_InitPPD, parent)
    {
    }

    int doWork();

    friend class DriverManager;
};

#endif //ZDRIVERMANAGER_P_H

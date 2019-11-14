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

#include "printertestpagedialog.h"
#include "ztroubleshoot.h"
#include "zcupsmonitor.h"
#include "zjobmanager.h"

#include <QLabel>

PrinterTestPageDialog::PrinterTestPageDialog(const QString &printerName, QWidget *parent)
    :DDialog (parent),
      m_testJob(nullptr)
{
    m_printerName = printerName;

    setIcon(QIcon(":/images/dde-printer.svg"));

    setMessage(tr("Check for ") + m_printerName);
    addButton(tr("cancel"));
    addButton(tr("sure"), true);
    setFixedHeight(202);

    m_trobleShoot = new TroubleShoot(printerName, this);
    connect(m_trobleShoot, &TroubleShoot::signalUpdateProgress, this, &PrinterTestPageDialog::slotTroubleShootMessage);
    connect(m_trobleShoot, &TroubleShoot::signalStatus, this, &PrinterTestPageDialog::slotTroubleShootStatus);
    m_trobleShoot->start();
}

void PrinterTestPageDialog::slotPrintState(int state, const QString &message)
{
    if (TStat_Suc == state) {
        setButtonText(0, tr("No"));
        setButtonText(1, tr("Yes"));
        setMessage(tr("Print job is completed, please check the printer is print or not"));
    } else {
        setMessage(g_cupsMonitor->getStateString(state) + " " + message);
    }
}

void PrinterTestPageDialog::slotTroubleShootMessage(int proccess, QString messge)
{
    Q_UNUSED(proccess);

    if (!messge.isEmpty())
        setMessage(messge);
}

void PrinterTestPageDialog::slotTroubleShootStatus(int id, int state)
{
    Q_UNUSED(id);

    if (TStat_Suc == state) {
        setMessage(tr("Start printer test page"));

        g_jobManager->holdjobs(m_printerName);
        m_testJob = new PrinterTestJob(m_printerName, this, false);
        connect(m_testJob, &PrinterTestJob::signalStateChanged, this, &PrinterTestPageDialog::slotPrintState);
        m_testJob->isPass();
    }
}

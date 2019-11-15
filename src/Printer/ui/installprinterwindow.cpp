/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     liurui <liurui_cm@deepin.com>
 *
 * Maintainer: liurui <liurui_cm@deepin.com>
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
#include "installprinterwindow.h"
#include "zjobmanager.h"
#include "addprinter.h"
#include "printersearchwindow.h"
#include "installdriverwindow.h"
#include "troubleshootdialog.h"
#include "ztroubleshoot.h"
#include "printerservice.h"
#include "zdrivermanager.h"

#include <DTitlebar>
#include <DSpinner>
#include <DIconButton>
#include <DWidgetUtil>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QMapIterator>

InstallPrinterWindow::InstallPrinterWindow(QWidget *parennt)
    : DMainWindow(parennt)
    , m_pParentWidget(parennt)
    , m_bInstallFail(false)
    , m_testJob(nullptr)
{
    initUI();
    initConnections();
    setStatus(Installing);
    m_pAddPrinterTask = nullptr;
}

InstallPrinterWindow::~InstallPrinterWindow()
{
    if (m_pAddPrinterTask) {
        delete m_pAddPrinterTask;
        m_pAddPrinterTask = nullptr;
    }
}

void InstallPrinterWindow::setTask(AddPrinterTask *task)
{
    m_pAddPrinterTask = task;
}

void InstallPrinterWindow::initUI()
{
    titlebar()->setMenuVisible(false);
    titlebar()->setTitle("");
    titlebar()->setIcon(QIcon(":/images/dde-printer.svg"));
    // 去掉最大最小按钮
    setWindowFlags(windowFlags() & ~Qt::WindowMinMaxButtonsHint);
    setWindowModality(Qt::ApplicationModal);
    setAttribute(Qt::WA_DeleteOnClose);
    resize(682, 532);

    m_pSpinner = new DSpinner();
    m_pSpinner->setFixedSize(32, 32);
    m_pStatusImageLabel = new QLabel();
    m_pStatusImageLabel->setFixedSize(128, 128);
    m_pStatusImageLabel->setAlignment(Qt::AlignCenter);
    m_pStatusLabel = new QLabel(tr("Installing driver..."));
    QFont font;
    font.setBold(true);
    m_pStatusLabel->setFont(font);
    m_pStatusLabel->setAlignment(Qt::AlignCenter);
    m_pTipLabel = new QLabel("");
    m_pTipLabel->setAlignment(Qt::AlignCenter);
    m_pTipLabel->setWordWrap(true);

    m_pDriverCombo = new QComboBox();
    m_pDriverCombo->setMinimumSize(300, 36);
    m_pCancelInstallBtn = new QPushButton(tr("Cancel"));

    m_pCheckPrinterListBtn = new QPushButton(tr("View Printer"));
    m_pCheckPrinterListBtn->setFixedSize(170, 36);
    m_pPrinterTestPageBtn = new QPushButton(tr("Print Test Page"));
    m_pPrinterTestPageBtn->setFixedSize(170, 36);
    QHBoxLayout *pHLayout = new QHBoxLayout();
    pHLayout->setSpacing(20);
    pHLayout->addStretch();
    pHLayout->addWidget(m_pCheckPrinterListBtn);
    pHLayout->addWidget(m_pPrinterTestPageBtn);
    pHLayout->addStretch();


    QVBoxLayout *pMainLayout = new QVBoxLayout();
    pMainLayout->addWidget(m_pSpinner, 0, Qt::AlignCenter);
    pMainLayout->addWidget(m_pStatusImageLabel, 0, Qt::AlignCenter);

    pMainLayout->addWidget(m_pStatusLabel, 0, Qt::AlignCenter);
    pMainLayout->addWidget(m_pTipLabel);
    pMainLayout->addWidget(m_pDriverCombo, 0, Qt::AlignCenter);
    pMainLayout->addStretch();
//    pMainLayout->addSpacing(130);
    pMainLayout->addWidget(m_pCancelInstallBtn, 0, Qt::AlignCenter);
    pMainLayout->addLayout(pHLayout);
    pMainLayout->setContentsMargins(0, 66, 0, 20);

    QWidget *widget = new QWidget;
    widget->setLayout(pMainLayout);
    takeCentralWidget();
    setCentralWidget(widget);

    moveToCenter(this);
}

void InstallPrinterWindow::initConnections()
{
    connect(m_pCancelInstallBtn, &QPushButton::clicked, this, &InstallPrinterWindow::cancelBtnClickedSlot);
    connect(m_pCheckPrinterListBtn, &QPushButton::clicked, this, &InstallPrinterWindow::leftBtnClickedSlot);
    connect(m_pPrinterTestPageBtn, &QPushButton::clicked, this, &InstallPrinterWindow::rightBtnClickedSlot);
}

void InstallPrinterWindow::setStatus(InstallationStatus status)
{
    if (m_status != status) {
        m_status = status;
        if (m_status == Installing) {
            m_pSpinner->setVisible(true);
            m_pSpinner->start();
            m_pStatusLabel->setVisible(true);
            m_pStatusLabel->setText(tr("Installing driver..."));
            m_pCancelInstallBtn->setVisible(true);

            m_pStatusImageLabel->setVisible(false);
            m_pTipLabel->setVisible(false);
            m_pDriverCombo->setVisible(false);
            m_pCheckPrinterListBtn->setVisible(false);
            m_pPrinterTestPageBtn->setVisible(false);
        } else if (m_status == Installed) {
            m_pSpinner->setVisible(false);
            m_pSpinner->stop();
            m_pStatusLabel->setVisible(true);
            m_pStatusLabel->setText(tr("Successfully installed") + m_printerName);
            m_pCancelInstallBtn->setVisible(false);

            m_pStatusImageLabel->setVisible(true);
            m_pStatusImageLabel->setPixmap(QPixmap(":/images/success.svg"));
            m_pTipLabel->setVisible(true);
            m_pTipLabel->setText(tr("You have successfully added the printer."
                                    "Print a test page to check if it works properly"));
            m_pDriverCombo->setVisible(false);
            m_pCheckPrinterListBtn->setVisible(true);
            m_pCheckPrinterListBtn->setText(tr("View Printer"));
            m_pPrinterTestPageBtn->setVisible(true);
            m_pPrinterTestPageBtn->setText(tr("Print Test Page"));
        } else if (m_status == Printing) {
            m_pSpinner->setVisible(false);
            m_pSpinner->stop();
            m_pStatusLabel->setVisible(true);
            m_pStatusLabel->setText(tr("Printing test page..."));
            m_pCancelInstallBtn->setVisible(false);

            m_pStatusImageLabel->setVisible(true);
            m_pStatusImageLabel->setPixmap(QPixmap(":/images/success.svg"));
            m_pTipLabel->setVisible(true);
            m_pTipLabel->setText(tr("You have successfully added the printer. "
                                    "Print a test page to check if it works properly."));
            m_pDriverCombo->setVisible(false);
            m_pCheckPrinterListBtn->setVisible(true);
            m_pCheckPrinterListBtn->setText(tr("View Printer"));
            m_pPrinterTestPageBtn->setVisible(true);
            m_pPrinterTestPageBtn->setText(tr("Print Test Page"));
        } else if (m_status == Printed) {
            m_pSpinner->setVisible(false);
            m_pSpinner->stop();
            m_pStatusLabel->setVisible(true);
            m_pStatusLabel->setText(tr("Did you print the test page successfully?"));
            m_pCancelInstallBtn->setVisible(false);

            m_pStatusImageLabel->setVisible(true);
            m_pStatusImageLabel->setPixmap(QPixmap(":/images/success.svg"));
            m_pTipLabel->setVisible(true);
            m_pTipLabel->setText(tr("if succeeded, click Yes; if failed, click No."));
            m_pDriverCombo->setVisible(false);
            m_pCheckPrinterListBtn->setVisible(true);
            m_pCheckPrinterListBtn->setText(tr("No"));
            m_pPrinterTestPageBtn->setVisible(true);
            m_pPrinterTestPageBtn->setText(tr("Yes"));
        } else if (m_status == PrintFailed) {
            m_pSpinner->setVisible(false);
            m_pSpinner->stop();
            m_pStatusLabel->setVisible(true);
            m_pStatusLabel->setText(tr("Print failed"));
            m_pCancelInstallBtn->setVisible(false);

            m_pStatusImageLabel->setVisible(true);
            m_pStatusImageLabel->setPixmap(QPixmap(":/images/fail.svg"));
            m_pTipLabel->setVisible(true);
            m_pTipLabel->setText(tr("Click Reinstall to install the printer driver again,or click Troubleshoot to start troubleshooting."));
            m_pDriverCombo->setVisible(false);
            m_pCheckPrinterListBtn->setVisible(true);
            m_pCheckPrinterListBtn->setText(tr("Reinstall"));
            m_pPrinterTestPageBtn->setVisible(true);
            m_pPrinterTestPageBtn->setText(tr("Troubleshoot"));
        } else if (m_status == Reinstall) {
            m_pSpinner->setVisible(false);
            m_pSpinner->stop();
            m_pStatusLabel->setVisible(true);

            if (m_bInstallFail) {
                m_pStatusLabel->setText(tr("Installation failed"));
            } else {
                m_pStatusLabel->setText(tr("Print failed"));
            }

            m_pCancelInstallBtn->setVisible(false);
            m_pStatusImageLabel->setVisible(true);
            m_pStatusImageLabel->setPixmap(QPixmap(":/images/fail.svg"));
            m_pTipLabel->setVisible(true);
            if (m_pAddPrinterTask)
                m_pTipLabel->setText(m_pAddPrinterTask->getErrorMassge());
            m_pCheckPrinterListBtn->setVisible(true);
            m_pCheckPrinterListBtn->setText(tr("Reinstall"));
            m_pPrinterTestPageBtn->setVisible(true);
            m_pPrinterTestPageBtn->setText(tr("Troubleshoot"));
            if (m_pDriverCombo->count() == 0)
                m_pPrinterTestPageBtn->setEnabled(false);
            else {
                m_pPrinterTestPageBtn->setEnabled(true);
            }
        }
    }
}

void InstallPrinterWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    if (!m_printerName.isEmpty())
        emit updatePrinterList(m_printerName);
}

void InstallPrinterWindow::setDefaultPrinterName(const QString &name)
{
    m_printerName = name;
}

void InstallPrinterWindow::setDevice(const TDeviceInfo &device)
{
    m_device = device;
}

void InstallPrinterWindow::copyDriverData(QComboBox *source)
{
    if (source) {
        int count = source->count();
        if (count <= 1)
            return;
        for (int i = 0; i < count - 1; i++) {
            m_pDriverCombo->addItem(source->itemText(i), source->itemData(i));
        }
    }
}

void InstallPrinterWindow::copyDriverData(const QMap<QString, QVariant> &itemDataMap)
{
    QMapIterator<QString, QVariant> it(itemDataMap);
    while (it.hasNext()) {
        it.next();
        m_pDriverCombo->addItem(it.key(), it.value());
    }
}

void InstallPrinterWindow::cancelBtnClickedSlot()
{
    if (m_pAddPrinterTask) {
        m_pAddPrinterTask->stop();
        close();
    }
}

void InstallPrinterWindow::feedbackPrintTestPage()
{
    if (m_pAddPrinterTask) {
        QMap<QString, QVariant> driver = m_pAddPrinterTask->getDriverInfo();

        if (driver[SD_KEY_from].toInt() == PPDFrom_Server) {
            int sid = driver[SD_KEY_sid].toInt();
            QString strReason = m_testJob?m_testJob->getMessage():"User feedback";
            QString strFeedback = QString("Uri: %1, device: %2").arg(m_device.uriList.join(" "))
                    .arg(m_device.strDeviceId.isEmpty()?m_device.strMakeAndModel:m_device.strDeviceId);
            PrinterServerInterface* server = g_printerServer->feedbackResult(sid, false, strReason, strFeedback);
            if (server)
                server->postToServer();
        }
    }
}

void InstallPrinterWindow::leftBtnClickedSlot()
{
    if (m_status == Installed) {
        close();
    } else if (m_status == Printed) {
        feedbackPrintTestPage();
        setStatus(PrintFailed);
    } else if (m_status == PrintFailed) {
        if (m_pDriverCombo->count() == 0) {
            if (m_pParentWidget) {
                InstallDriverWindow *pParent = static_cast<InstallDriverWindow *>(m_pParentWidget);
                if (pParent) {
                    pParent->show();
                } else {
                    PrinterSearchWindow *pParentSearch = static_cast<PrinterSearchWindow *>(m_pParentWidget);
                    if (pParentSearch) {
                        pParentSearch->show();
                    }
                }
            }
            close();
        } else {
            setStatus(Reinstall);
        }
    } else if (m_status == Reinstall) {
        if (m_pParentWidget) {
            InstallDriverWindow *pParent = static_cast<InstallDriverWindow *>(m_pParentWidget);
            if (pParent) {
                pParent->show();
            } else {
                PrinterSearchWindow *pParentSearch = static_cast<PrinterSearchWindow *>(m_pParentWidget);
                if (pParentSearch) {
                    pParentSearch->show();
                }
            }
        }
        close();
    }
}

void InstallPrinterWindow::rightBtnClickedSlot()
{
    if (m_status == Installed) {
        // 打印测试页
        if (m_testJob) {
            m_testJob->stop();
            m_testJob->deleteLater();
        }
        m_testJob = new PrinterTestJob(m_printerName, this, false);
        m_testJob->isPass();
        setStatus(Printed);
    } else if (m_status == Printed) {
        close();
    } else if (m_status == PrintFailed) {
        TroubleShootDialog dlg(m_printerName, this);
        dlg.setModal(true);
        dlg.exec();
    } else if (m_status == Reinstall) {
        if (m_pDriverCombo->count() <= 0)
            return;
        setStatus(Installing);
        QMap<QString, QVariant> solution = m_pDriverCombo->currentData().value<QMap<QString, QVariant>>();
        m_pAddPrinterTask = g_addPrinterFactoty->createAddPrinterTask(m_device, solution);
        connect(m_pAddPrinterTask, &AddPrinterTask::signalStatus, this, &InstallPrinterWindow::receiveInstallationStatusSlot);
        m_pAddPrinterTask->doWork();
    }
}

void InstallPrinterWindow::receiveInstallationStatusSlot(int status)
{
    AddPrinterTask *task = static_cast<AddPrinterTask *>(sender());

    setDefaultPrinterName(task->getPrinterInfo().strName);
    if (status == TStat_Suc) {
        m_bInstallFail = false;
        setStatus(Installed);
    } else {
        m_bInstallFail = true;
        setStatus(Reinstall);
        m_printerName.clear();
    }
}

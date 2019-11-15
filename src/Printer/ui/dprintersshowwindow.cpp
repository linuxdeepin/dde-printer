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
#include "dprintersshowwindow.h"
#include "zdrivermanager.h"
#include "dpropertysetdlg.h"
#include "printerapplication.h"
#include "connectedtask.h"
#include "qtconvert.h"
#include "zjobmanager.h"
#include "uisourcestring.h"
#include "zcupsmonitor.h"
#include "printertestpagedialog.h"
#include "troubleshootdialog.h"

#include <DDialog>
#include <DMessageBox>
#include <DFontSizeManager>
#include <DImageButton>
#include <DSettingsDialog>
#include <DTitlebar>
#include <DApplication>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QListWidget>
#include <QDebug>
#include <QMenu>
#include <QLineEdit>
#include <QCheckBox>

DPrintersShowWindow::DPrintersShowWindow(QWidget *parent)
    : DMainWindow(parent)
{
    m_pPrinterManager = DPrinterManager::getInstance();

    initUI();

    initConnections();
}

DPrintersShowWindow::~DPrintersShowWindow()
{

}

void DPrintersShowWindow::initUI()
{
    titlebar()->setTitle("");
    titlebar()->setIcon(QIcon(":/images/dde-printer.svg"));
    QMenu *pMenu = new QMenu();
    m_pSettings = new QAction(tr("Settings"));
    pMenu->addAction(m_pSettings);
    titlebar()->setMenu(pMenu);
    resize(942, 656);
    QFont font;
    font.setBold(true);
    // 左边上面的控制栏
    QLabel *pLabel = new QLabel(tr("Printers"));
    pLabel->setFont(font);
    m_pBtnAddPrinter = new DIconButton(DStyle::SP_IncreaseElement);
    m_pBtnAddPrinter->setFixedSize(36, 36);
    m_pBtnAddPrinter->setToolTip(tr("Add printer"));
    m_pBtnDeletePrinter = new DIconButton(DStyle::SP_DecreaseElement);
    m_pBtnDeletePrinter->setFixedSize(36, 36);
    m_pBtnDeletePrinter->setToolTip(tr("Delete printer"));
    QHBoxLayout *pLeftTopHLayout = new QHBoxLayout();
    pLeftTopHLayout->addWidget(pLabel, 6, Qt::AlignLeft);
    pLeftTopHLayout->addWidget(m_pBtnAddPrinter, 1);
    pLeftTopHLayout->addWidget(m_pBtnDeletePrinter, 1);
    // 坐标下面的列表
    m_pPrinterListView = new QListWidget();
    m_pPrinterListView->setTextElideMode(Qt::ElideRight);
    m_pPrinterListView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_pPrinterListView->setMinimumWidth(310);
//    m_pPrinterListView->itemDelegate()->installEventFilter(this);
    // 列表的右键菜单
    m_pListViewMenu = new QMenu();
    m_pShareAction = new QAction(tr("Shared"), m_pListViewMenu);
    m_pShareAction->setObjectName("Share");
    m_pShareAction->setCheckable(true);
    m_pEnableAction = new QAction(tr("Enabled"), m_pListViewMenu);
    m_pEnableAction->setObjectName("Enable");
    m_pEnableAction->setCheckable(true);
    m_pRejectAction = new QAction(tr("Accept Task"), m_pListViewMenu);
    m_pRejectAction->setObjectName("Accept");
    m_pRejectAction->setCheckable(true);

    m_pDefaultAction = new QAction(tr("Set as default"), m_pListViewMenu);
    m_pDefaultAction->setObjectName("Default");
    m_pDefaultAction->setCheckable(true);

    m_pListViewMenu->addAction(m_pShareAction);
    m_pListViewMenu->addAction(m_pEnableAction);
    m_pListViewMenu->addAction(m_pRejectAction);
    m_pListViewMenu->addSeparator();
    m_pListViewMenu->addAction(m_pDefaultAction);


    // 没有打印机时的提示
    m_pLeftTipLabel = new QLabel(tr("No Printers"));
    m_pLeftTipLabel->setVisible(false);
    m_pLeftTipLabel->setFont(font);
    // 左侧布局
    QVBoxLayout *pLeftVLayout = new QVBoxLayout();
    pLeftVLayout->addLayout(pLeftTopHLayout, 1);
    pLeftVLayout->addWidget(m_pPrinterListView, 4);
    pLeftVLayout->addWidget(m_pLeftTipLabel, 1, Qt::AlignCenter);

    // 右侧上方
    QLabel *pLabelImage = new QLabel("");
    pLabelImage->setPixmap(QPixmap(":/images/printer_details.svg"));

    m_pLabelPrinterName = new QLabel("") ;
    DFontSizeManager::instance()->bind(m_pLabelPrinterName, DFontSizeManager::T7);
    m_pLabelPrinterName->setFont(font);

    QLabel *pLabelLocation = new QLabel(tr("Location:"));
    pLabelLocation->setFixedWidth(60);
    m_pLabelLocationShow = new QLabel(tr(""));
    QLabel *pLabelType = new QLabel(tr("Model:"));
    m_pLabelTypeShow = new QLabel(tr(""));
    QLabel *pLabelStatus = new QLabel(tr("Status:"));
    m_pLabelStatusShow = new QLabel(tr(""));

    QGridLayout *pRightGridLayout = new QGridLayout();
    pRightGridLayout->addWidget(m_pLabelPrinterName, 0, 0, 1, 2, Qt::AlignLeft);
    pRightGridLayout->addWidget(pLabelLocation, 1, 0);
    pRightGridLayout->addWidget(m_pLabelLocationShow, 1, 1);
    pRightGridLayout->addWidget(pLabelType, 2, 0);
    pRightGridLayout->addWidget(m_pLabelTypeShow, 2, 1);
    pRightGridLayout->addWidget(pLabelStatus, 3, 0);
    pRightGridLayout->addWidget(m_pLabelStatusShow, 3, 1);
    pRightGridLayout->setColumnStretch(0, 1);
    pRightGridLayout->setColumnStretch(1, 2);

    QHBoxLayout *pRightTopHLayout = new QHBoxLayout();
    pRightTopHLayout->addWidget(pLabelImage, 0, Qt::AlignHCenter);
    pRightTopHLayout->addSpacing(60);
    pRightTopHLayout->addLayout(pRightGridLayout);

    // 右侧下方控件
    m_pTBtnSetting = new DIconButton(this);
    m_pTBtnSetting->setIcon(QIcon::fromTheme("dp_set"));
    m_pTBtnSetting->setIconSize(QSize(32, 32));

    QLabel *pLabelSetting = new QLabel();
    pLabelSetting->setText(tr("Settings"));
    pLabelSetting->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    m_pTBtnPrintQueue = new DIconButton(this);
    m_pTBtnPrintQueue->setIcon(QIcon::fromTheme("dp_print_queue"));
    m_pTBtnPrintQueue->setIconSize(QSize(32, 32));
    QLabel *pLabelPrintQueue = new QLabel();
    pLabelPrintQueue->setText(tr("Print Queue"));
    pLabelPrintQueue->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    m_pTBtnPrintTest = new DIconButton(this);
    m_pTBtnPrintTest->setIcon(QIcon::fromTheme("dp_test_page"));
    m_pTBtnPrintTest->setIconSize(QSize(32, 32));
    QLabel *pLabelPrintTest = new QLabel();
    pLabelPrintTest->setText(tr("Print Test Page"));
    pLabelPrintTest->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    m_pTBtnFault = new DIconButton(this);
    m_pTBtnFault->setIcon(QIcon::fromTheme("dp_fault"));
    m_pTBtnFault->setIconSize(QSize(32, 32));
    QLabel *pLabelPrintFault = new QLabel();
    pLabelPrintFault->setText(UI_PRINTERSHOW_TROUBLE);
    pLabelPrintFault->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    QGridLayout *pRightBottomGLayout = new QGridLayout();
    pRightBottomGLayout->addWidget(m_pTBtnSetting, 0, 0, Qt::AlignHCenter);
    pRightBottomGLayout->addWidget(pLabelSetting, 1, 0);
    pRightBottomGLayout->addWidget(m_pTBtnPrintQueue, 0, 1, Qt::AlignHCenter);
    pRightBottomGLayout->addWidget(pLabelPrintQueue, 1, 1);
    pRightBottomGLayout->addWidget(m_pTBtnPrintTest, 0, 2, Qt::AlignHCenter);
    pRightBottomGLayout->addWidget(pLabelPrintTest, 1, 2);
    pRightBottomGLayout->addWidget(m_pTBtnFault, 0, 3, Qt::AlignHCenter);
    pRightBottomGLayout->addWidget(pLabelPrintFault, 1, 3);

    // 右侧整体布局
    QVBoxLayout *pRightVLayout = new QVBoxLayout();
    pRightVLayout->addLayout(pRightTopHLayout);
    pRightVLayout->addSpacing(100);
    pRightVLayout->addLayout(pRightBottomGLayout);
    pRightVLayout->setContentsMargins(100, 100, 10, 0);

    m_pPrinterInfoWidget = new QWidget();
    m_pPrinterInfoWidget->setLayout(pRightVLayout);
    m_pPRightTipLabel = new QLabel(tr("No printer configured \nClick + to add printers"));
    m_pPRightTipLabel->setAlignment(Qt::AlignCenter);
    m_pPRightTipLabel->setVisible(false);
    m_pPRightTipLabel->setFont(font);
    QVBoxLayout *pRightMainVLayout = new QVBoxLayout();
    pRightMainVLayout->addWidget(m_pPrinterInfoWidget);
    pRightMainVLayout->addWidget(m_pPRightTipLabel);

    QWidget *pCentralWidget = new QWidget(this);
    QHBoxLayout *pMainHLayout = new QHBoxLayout();
    pMainHLayout->addLayout(pLeftVLayout, 1);
    pMainHLayout->addLayout(pRightMainVLayout, 2);
    pMainHLayout->setContentsMargins(10, 10, 0, 0);
    pCentralWidget->setLayout(pMainHLayout);
    takeCentralWidget();
    setCentralWidget(pCentralWidget);

    m_pSearchWindow = new PrinterSearchWindow();

    //设置对话框
    m_pSettingsDialog = new DDialog();
    m_pSettingsDialog->setIcon(QIcon(":/images/dde-printer.svg"));
    QWidget *pSettingWidget = new QWidget();
    QLabel *pBaseSettings = new QLabel(tr("Basic server Settings"));
    pBaseSettings->setFont(font);
    m_pCheckShared = new QCheckBox(tr("Publish Shared printers linked to this system"));
    m_pCheckIPP = new QCheckBox(tr("Allow printing from the Internet"));
    m_pCheckIPP->setEnabled(false);
//    m_pCheckRemote = new QCheckBox(tr("Allow remote administration"));
    m_pCheckCancelJobs = new QCheckBox(tr("Allow users to cancel all tasks (not just their own)"));
    m_pCheckSaveDebugInfo = new QCheckBox(tr("Retain debugging information for troubleshooting"));
    QVBoxLayout *pSettingsVLayout = new QVBoxLayout();
    pSettingsVLayout->addWidget(pBaseSettings);
    pSettingsVLayout->addWidget(m_pCheckShared);
    QHBoxLayout *pSettingsHLayout = new QHBoxLayout();
    pSettingsHLayout->addSpacing(20);
    pSettingsHLayout->addWidget(m_pCheckIPP);
    pSettingsVLayout->addLayout(pSettingsHLayout);
//    pSettingsVLayout->addWidget(m_pCheckRemote);
    pSettingsVLayout->addWidget(m_pCheckCancelJobs);
    pSettingsVLayout->addWidget(m_pCheckSaveDebugInfo);
    pSettingsVLayout->setSpacing(20);
    pSettingWidget->setLayout(pSettingsVLayout);
    m_pSettingsDialog->addContent(pSettingWidget);
}

void DPrintersShowWindow::initConnections()
{
    connect(m_pBtnAddPrinter, &DIconButton::clicked, this, &DPrintersShowWindow::addPrinterClickSlot);
    connect(m_pBtnDeletePrinter, &DIconButton::clicked, this, &DPrintersShowWindow::deletePrinterClickSlot);

    connect(m_pTBtnSetting, &DIconButton::clicked, this, &DPrintersShowWindow::printSettingClickSlot);
    connect(m_pTBtnPrintQueue, &DIconButton::clicked, this, &DPrintersShowWindow::printQueueClickSlot);
    connect(m_pTBtnPrintTest, &DIconButton::clicked, this, &DPrintersShowWindow::printTestClickSlot);
    connect(m_pTBtnFault, &DIconButton::clicked, this, &DPrintersShowWindow::printFalutClickSlot);

    connect(m_pPrinterListView, &QListWidget::currentRowChanged, this, &DPrintersShowWindow::printerListWidgetItemChangedSlot);
    //此处修改文字和修改图标都会触发这个信号，导致bug，修改图标之前先屏蔽信号
    connect(m_pPrinterListView, &QListWidget::itemChanged, this, &DPrintersShowWindow::renamePrinterSlot);
    connect(m_pPrinterListView, &QListWidget::customContextMenuRequested, this, &DPrintersShowWindow::contextMenuRequested);
    connect(m_pPrinterListView, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem * pItem) {
        // 存在未完成的任务无法进入编辑状态
        if (m_pPrinterManager->hasUnfinishedJob()) {
            pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
        } else {
            pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
        }
    });

    connect(m_pShareAction, &QAction::triggered, this, &DPrintersShowWindow::listWidgetMenuActionSlot);
    connect(m_pEnableAction, &QAction::triggered, this, &DPrintersShowWindow::listWidgetMenuActionSlot);
    connect(m_pDefaultAction, &QAction::triggered, this, &DPrintersShowWindow::listWidgetMenuActionSlot);
    connect(m_pRejectAction, &QAction::triggered, this, &DPrintersShowWindow::listWidgetMenuActionSlot);

    connect(m_pSearchWindow, &PrinterSearchWindow::updatePrinterList, this, &DPrintersShowWindow::reflushPrinterListView);

    connect(m_pCheckShared, &QCheckBox::clicked, this, [this](bool checked) {
        if (checked) {
            m_pCheckIPP->setEnabled(true);
        } else {
            m_pCheckIPP->setEnabled(false);
            m_pCheckIPP->setChecked(false);
        }
    });
    connect(m_pSettings, &QAction::triggered, this, &DPrintersShowWindow::serverSettingsSlot);

    connect(g_cupsMonitor, &CupsMonitor::signalPrinterStateChanged, this, [this](const QString & printer, int state, const QString & message) {
        Q_UNUSED(message)
        if (printer == m_CurPrinterName) {
            QString stateStr;
            if (state == 3) {
                stateStr = tr("Idle");
            } else if (state == 4) {
                stateStr = tr("printing");
            } else {
                stateStr = tr("Stopped");
            }
        }
    });
}

void DPrintersShowWindow::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    reflushPrinterListView(QString());
}

void DPrintersShowWindow::selectPrinterByName(const QString &printerName)
{
    int rowCount = m_pPrinterListView->count();
    for (int i = 0; i < rowCount; ++i) {
        if (m_pPrinterListView->item(i)->text() == printerName) {
            m_pPrinterListView->setCurrentRow(i);
        }
    }
}

void DPrintersShowWindow::updateDefaultPrinterIcon()
{
    //防止触发itemChanged信号
    m_pPrinterListView->blockSignals(true);
    int count = m_pPrinterListView->count();
    for (int i = 0; i < count; ++i) {
        if (m_pPrinterManager->isDefaultPrinter(m_pPrinterListView->item(i)->text())) {
            m_pPrinterListView->item(i)->setIcon(QIcon::fromTheme("dp_printer_default"));
        } else {
            m_pPrinterListView->item(i)->setIcon(QIcon::fromTheme("dp_printer_list"));
        }
    }
    m_pPrinterListView->blockSignals(false);
}


void DPrintersShowWindow::reflushPrinterListView(const QString &newPrinterName)
{
    m_pPrinterManager->updateDestinationList();
    m_pPrinterListView->clear();
    QStringList printerList = m_pPrinterManager->getPrintersList();
    foreach (QString printerName, printerList) {
        QListWidgetItem *pItem = new QListWidgetItem(printerName);
        pItem->setSizeHint(QSize(300, 50));
        pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
        pItem->setToolTip(printerName);
        if (m_pPrinterManager->isDefaultPrinter(printerName))
            pItem->setIcon(QIcon::fromTheme("dp_printer_default"));
        else {
            pItem->setIcon(QIcon::fromTheme("dp_printer_list"));
        }
        m_pPrinterListView->addItem(pItem);
    }
    if (m_pPrinterListView->count() > 0) {
        m_pPrinterListView->setVisible(true);
        m_pLeftTipLabel->setVisible(false);

        m_pPrinterInfoWidget->setVisible(true);
        m_pPRightTipLabel->setVisible(false);

        m_pBtnDeletePrinter->setEnabled(true);
    } else {
        m_pPrinterListView->setVisible(false);
        m_pLeftTipLabel->setVisible(true);
        m_pPrinterInfoWidget->setVisible(false);
        m_pPRightTipLabel->setVisible(true);
        m_pBtnDeletePrinter->setEnabled(false);
    }
    m_pPrinterListView->setFocusPolicy(Qt::NoFocus);

    if (newPrinterName.isEmpty()) {
        if (m_pPrinterListView->count() > 0) {
            m_pPrinterListView->setCurrentRow(0);
        }
    } else {
        selectPrinterByName(newPrinterName);
    }
}

void DPrintersShowWindow::serverSettingsSlot()
{
    if (m_pPrinterManager->isSharePrintersEnabled()) {
        m_pCheckShared->setChecked(true);
        m_pCheckIPP->setChecked(m_pPrinterManager->isRemoteAnyEnabled());
        m_pCheckIPP->setEnabled(true);
    } else {
        m_pCheckShared->setChecked(false);
        m_pCheckIPP->setChecked(false);
        m_pCheckIPP->setEnabled(false);
    }
//        m_pCheckRemote->setChecked(m_pPrinterManager->isRemoteAdminEnabled());
    m_pCheckCancelJobs->setChecked(m_pPrinterManager->isUserCancelAnyEnabled());
    m_pCheckSaveDebugInfo->setChecked(m_pPrinterManager->isDebugLoggingEnabled());
    m_pSettingsDialog->exec();
    if (m_pCheckShared->isChecked()) {
        m_pPrinterManager->enableSharePrinters(true);
        m_pPrinterManager->enableRemoteAny(m_pCheckIPP->isChecked());
    } else {
        m_pPrinterManager->enableSharePrinters(false);
        m_pPrinterManager->enableRemoteAny(false);
    }
//        m_pPrinterManager->enableRemoteAdmin(m_pCheckRemote);
    m_pPrinterManager->enableUserCancelAny(m_pCheckCancelJobs->isChecked());
    m_pPrinterManager->enableDebugLogging(m_pCheckSaveDebugInfo->isChecked());
    m_pPrinterManager->commit();
}

void DPrintersShowWindow::addPrinterClickSlot()
{
    if (m_pSearchWindow)
        m_pSearchWindow->show();
}

void DPrintersShowWindow::deletePrinterClickSlot()
{
    if (!m_pPrinterListView->currentItem())
        return;
    QString printerName = m_pPrinterListView->currentItem()->text();
    if (!printerName.isEmpty()) {
        DDialog *pDialog = new DDialog(this);
        QLabel *pMessage = new QLabel(tr("Are you sure you want to delete the printer \"%1\" ?").arg(printerName));
        pMessage->setWordWrap(true);
        pMessage->setAlignment(Qt::AlignCenter);
        pDialog->addContent(pMessage);
        pDialog->addButton(UI_PRINTERSHOW_CANCEL);
        pDialog->addSpacing(20);
        pDialog->addButton(tr("Delete printer"), true, DDialog::ButtonType::ButtonWarning);
        pDialog->setIcon(QIcon(":/images/warning_logo.svg"));
        int ret = pDialog->exec();
        if (ret > 0) {
            bool suc = m_pPrinterManager->deletePrinterByName(printerName);
            if (suc) {
                // 刷新打印机列表
                m_CurPrinterName = "";
                reflushPrinterListView(m_CurPrinterName);
            }
        }
        pDialog->deleteLater();
    }
}

void DPrintersShowWindow::renamePrinterSlot(QListWidgetItem *pItem)
{
    //过滤掉空格
    QString newPrinterName = pItem->text().remove(" ");
    if (newPrinterName == m_CurPrinterName) {
        pItem->setText(m_CurPrinterName);
        return;
    }
    try {
        if (m_pPrinterManager->hasFinishedJob()) {
            DDialog *pDialog = new DDialog();
            pDialog->setIcon(QIcon(":/images/warning_logo.svg"));
            pDialog->setMessage(tr("Renaming will cause the completed task not to be reprinted, are you sure?"));
            pDialog->addButton(UI_PRINTERSHOW_CANCEL);
            int okIndex = pDialog->addButton(tr("Confirm"));

            int ret = pDialog->exec();
            pDialog->deleteLater();
            if (ret != okIndex) {
                //避免重复触发itemchanged信号
                m_pPrinterListView->blockSignals(true);
                pItem->setText(m_CurPrinterName);
                m_pPrinterListView->blockSignals(false);
                return;
            }
        }
        QString info, location, deviceURI, ppdFile;
        DDestination *pDest = m_pPrinterManager->getDestinationByName(m_CurPrinterName);
        if (!pDest)
            return;
        bool isDefault = m_pPrinterManager->isDefaultPrinter(m_CurPrinterName);
        bool isAccept = m_pPrinterManager->isPrinterAcceptJob(m_CurPrinterName);
        m_pPrinterManager->setPrinterEnabled(m_CurPrinterName, false);

        pDest->getPrinterInfo(info, location, deviceURI, ppdFile);
        m_pPrinterManager->addPrinter(newPrinterName, info, location, deviceURI, ppdFile);
        m_pPrinterManager->setPrinterEnabled(newPrinterName, true);
        if (isDefault)
            m_pPrinterManager->setPrinterDefault(newPrinterName);
        if (isAccept)
            m_pPrinterManager->setPrinterAcceptJob(newPrinterName, true);
        m_pPrinterManager->deletePrinterByName(m_CurPrinterName);

        m_CurPrinterName = newPrinterName;
        // 刷新完成选中重命名的打印机
        reflushPrinterListView(m_CurPrinterName);
    } catch (const std::runtime_error &e) {
        qWarning() << e.what();
    }
}

void DPrintersShowWindow::printSettingClickSlot()
{
    QListWidgetItem *pItem = m_pPrinterListView->currentItem();
    if (!pItem)
        return ;
    QString strPrinterName = pItem->text();
    DPropertySetDlg dlg(this);
    dlg.setPrinterName(strPrinterName);

    if (dlg.isDriveBroken()) {
        DDialog dialog;
        dialog.setFixedSize(QSize(400, 150));
        dialog.setMessage(tr("The driver is damaged, please install it again."));
        dialog.addSpacing(10);
        dialog.addButton(UI_PRINTERSHOW_CANCEL);
        int iIndex = dialog.addButton(tr("install driver"));
        dialog.setIconPixmap(QPixmap(":/images/warning_logo.svg"));
        QAbstractButton *pBtn = dialog.getButton(iIndex);
        connect(pBtn, &QAbstractButton::clicked, this, &DPrintersShowWindow::printDriveInstall);
        dialog.exec();
    } else {
        dlg.updateViews();
        dlg.exec();
    }
}

void DPrintersShowWindow::printQueueClickSlot()
{
    g_printerApplication->showJobsWindow();
}

void DPrintersShowWindow::printTestClickSlot()
{
    if (!m_pPrinterListView->currentItem())
        return;
    QString printerName = m_pPrinterListView->currentItem()->text();

    PrinterTestPageDialog dlg(printerName, this);
    dlg.setModal(true);
    dlg.exec();
}

void DPrintersShowWindow::printFalutClickSlot()
{
    if (!m_pPrinterListView->currentItem())
        return;
    QString printerName = m_pPrinterListView->currentItem()->text();

    TroubleShootDialog dlg(printerName, this);
    dlg.setModal(true);
    dlg.exec();
}

void DPrintersShowWindow::printDriveInstall()
{
    m_pSearchWindow->show();
}

void DPrintersShowWindow::printerListWidgetItemChangedSlot(int row)
{
    //在清空QListWidget的时候会触发这个槽函数，需要先判断是否这个item存在，不然会导致段错误
    if (!m_pPrinterListView->item(row))
        return;
    QString printerName = m_pPrinterListView->item(row)->text();
    m_CurPrinterName = printerName;
    QStringList basePrinterInfo = m_pPrinterManager->getPrinterBaseInfoByName(printerName);
    if (basePrinterInfo.count() == 3) {
        QString showPrinter = printerName;
        //如果文字超出了显示范围，那么就用省略号代替，并且设置tip提示
        geteElidedText(m_pLabelPrinterName->font(), showPrinter, int(180.0 / 942 * this->width()));
        m_pLabelPrinterName->setText(showPrinter);
        m_pLabelPrinterName->setToolTip(printerName);
        m_pLabelLocationShow->setText(basePrinterInfo.at(0));
        m_pLabelLocationShow->setToolTip(basePrinterInfo.at(0));
        m_pLabelTypeShow->setText(basePrinterInfo.at(1));
        m_pLabelTypeShow->setToolTip(basePrinterInfo.at(1));
        m_pLabelStatusShow->setText(basePrinterInfo.at(2));

        ConnectedTask *pTask = new ConnectedTask(printerName);
        connect(pTask, &ConnectedTask::signalResult, this, [&](bool connected, const QString & signalPrinterName) {
            if ((!connected) && (m_pPrinterListView->currentItem()->text() == signalPrinterName)) {
                m_pLabelStatusShow->setText(tr("disconnected"));
            }
        });
        //将线程对象的释放与更新状态分开
        connect(pTask, &ConnectedTask::finished, pTask, &ConnectedTask::deleteLater);
        pTask->start();
    }
}

void DPrintersShowWindow::contextMenuRequested(const QPoint &point)
{
    if (m_pPrinterListView->currentItem()) {
        bool isShared = m_pPrinterManager->isPrinterShared(m_pPrinterListView->currentItem()->text());
        m_pShareAction->setChecked(isShared);
        bool isEnabled = m_pPrinterManager->isPrinterEnabled(m_pPrinterListView->currentItem()->text());
        m_pEnableAction->setChecked(isEnabled);
        bool isAcceptJob = m_pPrinterManager->isPrinterAcceptJob(m_pPrinterListView->currentItem()->text());
        m_pRejectAction->setChecked(isAcceptJob);
        bool isDefault = m_pPrinterManager->isDefaultPrinter(m_pPrinterListView->currentItem()->text());
        m_pDefaultAction->setChecked(isDefault);
        // 当打印机为默认时，不让用户取消，避免出现没有默认打印机的状态
        m_pDefaultAction->setDisabled(isDefault);
        m_pListViewMenu->popup(mapToParent(point));
    }
}

void DPrintersShowWindow::listWidgetMenuActionSlot(bool checked)
{
    if (sender()->objectName() == "Share") {
        m_pPrinterManager->setPrinterShared(m_pPrinterListView->currentItem()->text(), checked);
    } else if (sender()->objectName() == "Enable") {
        m_pPrinterManager->setPrinterEnabled(m_pPrinterListView->currentItem()->text(), checked);
        //更新打印机状态信息
        printerListWidgetItemChangedSlot(m_pPrinterListView->currentRow());

    } else if (sender()->objectName() == "Default") {
        if (checked) {
            m_pPrinterManager->setPrinterDefault(m_pPrinterListView->currentItem()->text());
            updateDefaultPrinterIcon();
        }
    } else if (sender()->objectName() == "Accept") {
        m_pPrinterManager->setPrinterAcceptJob(m_pPrinterListView->currentItem()->text(), checked);
    }
}





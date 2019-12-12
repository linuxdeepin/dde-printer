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
#include "ztroubleshoot_p.h"

#include <DDialog>
#include <DMessageBox>
#include <DImageButton>
#include <DSettingsDialog>
#include <DTitlebar>
#include <DApplication>
#include <DFloatingButton>
#include <DFrame>
#include <DBackgroundGroup>
#include <DErrorMessage>


#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QListWidget>
#include <QDebug>
#include <QMenu>
#include <QLineEdit>
#include <QTimer>
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
    if (m_pSearchWindow)
        m_pSearchWindow->deleteLater();
    if (m_pSettingsDialog)
        m_pSettingsDialog->deleteLater();
}

void DPrintersShowWindow::initUI()
{
    titlebar()->setTitle("");
    titlebar()->setIcon(QIcon(":/images/dde-printer.svg"));
    QMenu *pMenu = new QMenu();
    m_pSettings = new QAction(tr("Settings"));
    pMenu->addAction(m_pSettings);
    titlebar()->setMenu(pMenu);
    setMinimumSize(942, 656);

    // 左边上面的控制栏
    QLabel *pLabel = new QLabel(tr("Printers"));
    DFontSizeManager::instance()->bind(pLabel, DFontSizeManager::T5, int(QFont::DemiBold));
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
    pLeftTopHLayout->setContentsMargins(0, 0, 0, 0);
    // 打印机列表
    m_pPrinterListView = new PrinterListView(this);
    m_pPrinterModel = new QStandardItemModel(m_pPrinterListView);
    m_pPrinterListView->setTextElideMode(Qt::ElideRight);
    m_pPrinterListView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_pPrinterListView->setMinimumWidth(310);
    m_pPrinterListView->setItemSpacing(10);
    m_pPrinterListView->setModel(m_pPrinterModel);

    // 列表的右键菜单
    m_pListViewMenu = new QMenu();
    m_pShareAction = new QAction(tr("Shared"), m_pListViewMenu);
    m_pShareAction->setObjectName("Share");
    m_pShareAction->setCheckable(true);
    m_pEnableAction = new QAction(tr("Enabled"), m_pListViewMenu);
    m_pEnableAction->setObjectName("Enable");
    m_pEnableAction->setCheckable(true);
    m_pRejectAction = new QAction(tr("Accept jobs"), m_pListViewMenu);
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
    QPalette pa = m_pLeftTipLabel->palette();
    QColor color = pa.color(QPalette::WindowText);
    pa.setColor(QPalette::WindowText, QColor(color.red(), color.green(), color.blue(), int(255 * 0.3)));
    m_pLeftTipLabel->setPalette(pa);
    DFontSizeManager::instance()->bind(m_pLeftTipLabel, DFontSizeManager::T5, QFont::DemiBold);
    // 左侧布局
    QVBoxLayout *pLeftVLayout = new QVBoxLayout();
    pLeftVLayout->addLayout(pLeftTopHLayout, 1);
    pLeftVLayout->addWidget(m_pPrinterListView, 4);
    pLeftVLayout->addWidget(m_pLeftTipLabel, 1, Qt::AlignCenter);
    pLeftVLayout->setContentsMargins(0, 0, 0, 0);
    QWidget *pLeftWidget = new QWidget(this);
    pLeftWidget->setLayout(pLeftVLayout);

    // 右侧上方
    QLabel *pLabelImage = new QLabel("");
    pLabelImage->setPixmap(QPixmap(":/images/printer_details.svg"));

    m_pLabelPrinterName = new QLabel("") ;
    m_pLabelPrinterName->setMinimumWidth(200);
    DFontSizeManager::instance()->bind(m_pLabelPrinterName, DFontSizeManager::T4, QFont::DemiBold);


    QLabel *pLabelLocation = new QLabel(tr("Location:"));
    pLabelLocation->setFixedWidth(60);
    m_pLabelLocationShow = new QLabel(tr(""));
    DFontSizeManager::instance()->bind(m_pLabelLocationShow, DFontSizeManager::T5, QFont::DemiBold);
    QLabel *pLabelType = new QLabel(tr("Model:"));
    m_pLabelTypeShow = new QLabel(tr(""));
    DFontSizeManager::instance()->bind(m_pLabelTypeShow, DFontSizeManager::T5, QFont::DemiBold);
    QLabel *pLabelStatus = new QLabel(tr("Status:"));
    m_pLabelStatusShow = new QLabel(tr(""));
    DFontSizeManager::instance()->bind(m_pLabelStatusShow, DFontSizeManager::T5, QFont::DemiBold);
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
    pRightTopHLayout->addSpacing(30);
    pRightTopHLayout->addLayout(pRightGridLayout);

    // 右侧下方控件
    m_pTBtnSetting = new DFloatingButton(this);
    m_pTBtnSetting->setIcon(QIcon::fromTheme("dp_set"));
    m_pTBtnSetting->setIconSize(QSize(32, 32));
    m_pTBtnSetting->setFixedSize(60, 60);
    m_pTBtnSetting->setBackgroundRole(QPalette::Button);

    QLabel *pLabelSetting = new QLabel();
    pLabelSetting->setText(tr("Properties"));
    pLabelSetting->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    DFontSizeManager::instance()->bind(pLabelSetting, DFontSizeManager::T8);

    m_pTBtnPrintQueue = new DFloatingButton(this);
    m_pTBtnPrintQueue->setIcon(QIcon::fromTheme("dp_print_queue"));
    m_pTBtnPrintQueue->setIconSize(QSize(32, 32));
    m_pTBtnPrintQueue->setFixedSize(60, 60);
    m_pTBtnPrintQueue->setBackgroundRole(QPalette::Button);
    QLabel *pLabelPrintQueue = new QLabel();
    pLabelPrintQueue->setText(tr("Print Queue"));
    pLabelPrintQueue->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    DFontSizeManager::instance()->bind(pLabelPrintQueue, DFontSizeManager::T8);

    m_pTBtnPrintTest = new DFloatingButton(this);
    m_pTBtnPrintTest->setIcon(QIcon::fromTheme("dp_test_page"));
    m_pTBtnPrintTest->setIconSize(QSize(32, 32));
    m_pTBtnPrintTest->setFixedSize(60, 60);
    m_pTBtnPrintTest->setBackgroundRole(QPalette::Button);
    QLabel *pLabelPrintTest = new QLabel();
    pLabelPrintTest->setText(tr("Print Test Page"));
    pLabelPrintTest->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    DFontSizeManager::instance()->bind(pLabelPrintTest, DFontSizeManager::T8);

    m_pTBtnFault = new DFloatingButton(this);
    m_pTBtnFault->setIcon(QIcon::fromTheme("dp_fault"));
    m_pTBtnFault->setIconSize(QSize(32, 32));
    m_pTBtnFault->setFixedSize(60, 60);
    m_pTBtnFault->setBackgroundRole(QPalette::Button);

    QLabel *pLabelPrintFault = new QLabel();
    pLabelPrintFault->setText(UI_PRINTERSHOW_TROUBLE);
    pLabelPrintFault->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    DFontSizeManager::instance()->bind(pLabelPrintFault, DFontSizeManager::T8);

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
    pRightVLayout->setContentsMargins(60, 100, 60, 181);

    m_pPrinterInfoWidget = new QWidget();
    m_pPrinterInfoWidget->setLayout(pRightVLayout);
    m_pPRightTipLabel1 = new QLabel(tr("No printer configured"));
    m_pPRightTipLabel1->setAlignment(Qt::AlignCenter);
    m_pPRightTipLabel1->setVisible(false);
    m_pPRightTipLabel1->setPalette(pa);
    DFontSizeManager::instance()->bind(m_pPRightTipLabel1, DFontSizeManager::T5, QFont::DemiBold);
    m_pPRightTipLabel2 = new QLabel(tr("Click + to add printers"));
    m_pPRightTipLabel2->setAlignment(Qt::AlignCenter);
    m_pPRightTipLabel2->setVisible(false);
    m_pPRightTipLabel2->setPalette(pa);
    DFontSizeManager::instance()->bind(m_pPRightTipLabel2, DFontSizeManager::T6, QFont::DemiBold);
    QVBoxLayout *pRightMainVLayout = new QVBoxLayout();
    pRightMainVLayout->addWidget(m_pPrinterInfoWidget);
    pRightMainVLayout->addStretch();
    pRightMainVLayout->addWidget(m_pPRightTipLabel1);
    pRightMainVLayout->addWidget(m_pPRightTipLabel2);
    pRightMainVLayout->addStretch();
    //DFrame 会导致上面的QLabel显示颜色不正常
    QWidget *pRightWidgwt = new QWidget();
    pRightWidgwt->setLayout(pRightMainVLayout);


    QHBoxLayout *pMainHLayout = new QHBoxLayout();
    pMainHLayout->setSpacing(2);
    pMainHLayout->addWidget(pLeftWidget, 1);
    pMainHLayout->addWidget(pRightWidgwt, 2);
//    pMainHLayout->setContentsMargins(10, 10, 10, 10);
    //阴影分割布局控件
    DBackgroundGroup *pCentralWidget = new DBackgroundGroup();
    pCentralWidget->setLayout(pMainHLayout);
    pCentralWidget->setItemSpacing(2);
    QHBoxLayout *pMainLayout1 = new QHBoxLayout();
    pMainLayout1->addWidget(pCentralWidget);
    pMainLayout1->setContentsMargins(10, 10, 10, 10);
    QWidget *pCentralWidget1 = new QWidget();
    pCentralWidget1->setLayout(pMainLayout1);
    takeCentralWidget();
    setCentralWidget(pCentralWidget1);
    //设置了parent会导致moveToCenter失效
    m_pSearchWindow = new PrinterSearchWindow();
    //设置对话框
    m_pSettingsDialog = new ServerSettingsWindow();
}

void DPrintersShowWindow::initConnections()
{
    connect(m_pBtnAddPrinter, &DIconButton::clicked, this, &DPrintersShowWindow::addPrinterClickSlot);
    connect(m_pBtnDeletePrinter, &DIconButton::clicked, this, &DPrintersShowWindow::deletePrinterClickSlot);

    connect(m_pTBtnSetting, &DIconButton::clicked, this, &DPrintersShowWindow::printSettingClickSlot);
    connect(m_pTBtnPrintQueue, &DIconButton::clicked, this, &DPrintersShowWindow::printQueueClickSlot);
    connect(m_pTBtnPrintTest, &DIconButton::clicked, this, &DPrintersShowWindow::printTestClickSlot);
    connect(m_pTBtnFault, &DIconButton::clicked, this, &DPrintersShowWindow::printFalutClickSlot);

    connect(m_pPrinterListView, QOverload<const QModelIndex &>::of(&DListView::currentChanged), this, &DPrintersShowWindow::printerListWidgetItemChangedSlot);
//    //此处修改文字和修改图标都会触发这个信号，导致bug，修改图标之前先屏蔽信号
    connect(m_pPrinterModel, &QStandardItemModel::itemChanged, this, &DPrintersShowWindow::renamePrinterSlot);
    connect(m_pPrinterListView, &DListView::customContextMenuRequested, this, &DPrintersShowWindow::contextMenuRequested);
    connect(m_pPrinterListView, &DListView::doubleClicked, this, [this](const QModelIndex & index) {
        // 存在未完成的任务无法进入编辑状态
        m_pPrinterListView->blockSignals(true);
        m_pPrinterModel->blockSignals(true);
        QStandardItem *pItem = m_pPrinterModel->itemFromIndex(index);
        if (m_pPrinterManager->hasUnfinishedJob()) {
            DDialog *pDialog = new DDialog();
            pDialog->setIcon(QIcon(":/images/warning_logo.svg"));
            QLabel *pMessage = new QLabel(tr("As print jobs are in process, you cannot rename the printer now, please try later"));
            pMessage->setWordWrap(true);
            pMessage->setAlignment(Qt::AlignCenter);
            pDialog->addContent(pMessage);
            pDialog->addButton(tr("OK"));
            pDialog->exec();
            pDialog->deleteLater();
            pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
        } else {
            pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
        }

        m_pPrinterListView->blockSignals(false);
        m_pPrinterModel->blockSignals(false);
    });

    connect(m_pShareAction, &QAction::triggered, this, &DPrintersShowWindow::listWidgetMenuActionSlot);
    connect(m_pEnableAction, &QAction::triggered, this, &DPrintersShowWindow::listWidgetMenuActionSlot);
    connect(m_pDefaultAction, &QAction::triggered, this, &DPrintersShowWindow::listWidgetMenuActionSlot);
    connect(m_pRejectAction, &QAction::triggered, this, &DPrintersShowWindow::listWidgetMenuActionSlot);

    connect(m_pSearchWindow, &PrinterSearchWindow::updatePrinterList, this, &DPrintersShowWindow::refreshPrinterListView);


    connect(m_pSettings, &QAction::triggered, this, &DPrintersShowWindow::serverSettingsSlot);

    connect(g_cupsMonitor, &CupsMonitor::signalPrinterStateChanged, this, [this](const QString & printer, int state, const QString & message) {
        Q_UNUSED(message)
        if (printer == m_CurPrinterName) {
            QString stateStr;
            if (state == 3) {
                stateStr = tr("Idle");
            } else if (state == 4) {
                stateStr = tr("Printing");
            } else {
                stateStr = tr("Disabled");
            }
        }
    });
}

void DPrintersShowWindow::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    refreshPrinterListView(QString());

    QTimer::singleShot(10, this, [ = ]() {
        CheckCupsServer cups(this);
        if (!cups.isPass()) {
            DDialog dlg("", tr("CUPS server is not running, and can’t manage printers."));

            dlg.setIcon(QIcon(":/images/warning_logo.svg"));
            dlg.addButton(tr("OK"), true);
            dlg.setContentsMargins(10, 15, 10, 15);
            dlg.setModal(true);
            dlg.setFixedSize(422, 202);
            dlg.exec();
        }
    });
}

void DPrintersShowWindow::selectPrinterByName(const QString &printerName)
{
    int rowCount = m_pPrinterModel->rowCount();
    for (int i = 0; i < rowCount; ++i) {
        if (m_pPrinterModel->item(i)->text() == printerName) {
            m_pPrinterListView->setCurrentIndex(m_pPrinterModel->item(i)->index());
        }
    }
}

void DPrintersShowWindow::updateDefaultPrinterIcon()
{
    //防止触发itemChanged信号
    m_pPrinterListView->blockSignals(true);
    m_pPrinterModel->blockSignals(true);
    int count = m_pPrinterModel->rowCount();
    for (int i = 0; i < count; ++i) {
        if (m_pPrinterManager->isDefaultPrinter(m_pPrinterModel->item(i)->text())) {
            m_pPrinterModel->item(i)->setIcon(QIcon::fromTheme("dp_printer_default"));
        } else {
            m_pPrinterModel->item(i)->setIcon(QIcon::fromTheme("dp_printer_list"));
        }
    }
    m_pPrinterListView->blockSignals(false);
    m_pPrinterModel->blockSignals(false);
}



void DPrintersShowWindow::refreshPrinterListView(const QString &newPrinterName)
{
    m_pPrinterManager->updateDestinationList();
    m_pPrinterModel->clear();
    QStringList printerList = m_pPrinterManager->getPrintersList();
    foreach (QString printerName, printerList) {
        DStandardItem *pItem = new DStandardItem(printerName);
        pItem->setData(VListViewItemMargin, Dtk::MarginsRole);
        pItem->setSizeHint(QSize(300, 50));
        pItem->setToolTip(printerName);
        if (m_pPrinterManager->isDefaultPrinter(printerName))
            pItem->setIcon(QIcon::fromTheme("dp_printer_default"));
        else {
            pItem->setIcon(QIcon::fromTheme("dp_printer_list"));
        }
        m_pPrinterModel->appendRow(pItem);
    }
    if (m_pPrinterListView->count() > 0) {
        m_pPrinterListView->setVisible(true);
        m_pLeftTipLabel->setVisible(false);

        m_pPrinterInfoWidget->setVisible(true);
        m_pPRightTipLabel1->setVisible(false);
        m_pPRightTipLabel2->setVisible(false);

        m_pBtnDeletePrinter->setEnabled(true);
    } else {
        m_pPrinterListView->setVisible(false);
        m_pLeftTipLabel->setVisible(true);
        m_pPrinterInfoWidget->setVisible(false);
        m_pPRightTipLabel1->setVisible(true);
        m_pPRightTipLabel2->setVisible(true);
        m_pBtnDeletePrinter->setEnabled(false);
    }
    m_pPrinterListView->setFocusPolicy(Qt::NoFocus);

    if (newPrinterName.isEmpty()) {
        if (m_pPrinterListView->count() > 0) {
            m_pPrinterListView->setCurrentIndex(m_pPrinterModel->index(0, 0));
        }
    } else {
        selectPrinterByName(newPrinterName);
    }
}

void DPrintersShowWindow::serverSettingsSlot()
{
    if (m_pPrinterManager->isSharePrintersEnabled()) {
        m_pSettingsDialog->m_pCheckShared->setChecked(true);
        m_pSettingsDialog->m_pCheckIPP->setChecked(m_pPrinterManager->isRemoteAnyEnabled());
        m_pSettingsDialog->m_pCheckIPP->setEnabled(true);
    } else {
        m_pSettingsDialog->m_pCheckShared->setChecked(false);
        m_pSettingsDialog->m_pCheckIPP->setChecked(false);
        m_pSettingsDialog->m_pCheckIPP->setEnabled(false);
    }
    m_pSettingsDialog->m_pCheckRemote->setChecked(m_pPrinterManager->isRemoteAdminEnabled());
//    m_pSettingsDialog->m_pCheckCancelJobs->setChecked(m_pPrinterManager->isUserCancelAnyEnabled());
    m_pSettingsDialog->m_pCheckSaveDebugInfo->setChecked(m_pPrinterManager->isDebugLoggingEnabled());
    m_pSettingsDialog->exec();
    if (m_pSettingsDialog->m_pCheckShared->isChecked()) {
        m_pPrinterManager->enableSharePrinters(true);
        m_pPrinterManager->enableRemoteAny(m_pSettingsDialog->m_pCheckIPP->isChecked());
    } else {
        m_pPrinterManager->enableSharePrinters(false);
        m_pPrinterManager->enableRemoteAny(false);
    }
    m_pPrinterManager->enableRemoteAdmin(m_pSettingsDialog->m_pCheckRemote->isChecked());
//    m_pPrinterManager->enableUserCancelAny(m_pSettingsDialog->m_pCheckCancelJobs->isChecked());
    m_pPrinterManager->enableDebugLogging(m_pSettingsDialog->m_pCheckSaveDebugInfo->isChecked());
    m_pPrinterManager->commit();
}

//bool DPrintersShowWindow::eventFilter(QObject *watched, QEvent *event)
//{
//    if (watched == m_pPrinterListView) {
//        if (event->type() == QEvent::MouseButtonRelease) {
//            if (m_pPrinterListView->isPersistentEditorOpen(m_pPrinterListView->currentIndex())) {
//                m_pPrinterListView->closePersistentEditor(m_pPrinterListView->currentIndex());
//            }
//        }
//    }
//    return false;
//}

void DPrintersShowWindow::closeEvent(QCloseEvent *event)
{
    DMainWindow::closeEvent(event);

    QTimer::singleShot(10, g_printerApplication, &PrinterApplication::slotMainWindowClosed);
}

void DPrintersShowWindow::addPrinterClickSlot()
{
    if (m_pSearchWindow)
        m_pSearchWindow->show();
}

void DPrintersShowWindow::deletePrinterClickSlot()
{
    if (!m_pPrinterListView->currentIndex().isValid())
        return;
    QString printerName = m_pPrinterListView->currentIndex().data().toString();
    if (!printerName.isEmpty()) {
        DDialog *pDialog = new DDialog(this);
        QLabel *pMessage = new QLabel(tr("Are you sure you want to delete the printer \"%1\" ?").arg(printerName));
        pMessage->setWordWrap(true);
        pMessage->setAlignment(Qt::AlignCenter);
        pDialog->addContent(pMessage);
        pDialog->addButton(UI_PRINTERSHOW_CANCEL);
        pDialog->addSpacing(20);
        pDialog->addButton(tr("Delete"), true, DDialog::ButtonType::ButtonWarning);
        pDialog->setIcon(QIcon(":/images/warning_logo.svg"));
        int ret = pDialog->exec();
        if (ret > 0) {
            bool suc = m_pPrinterManager->deletePrinterByName(printerName);
            if (suc) {
                // 刷新打印机列表
                m_CurPrinterName = "";
                refreshPrinterListView(m_CurPrinterName);
            }
        }
        pDialog->deleteLater();
    }
}

void DPrintersShowWindow::renamePrinterSlot(QStandardItem *pItem)
{
    //过滤掉空格
    if (!pItem)
        return;
    QString newPrinterName = m_pPrinterManager->validataName(pItem->text());
    if (newPrinterName.isEmpty()) {
        m_pPrinterListView->blockSignals(true);
        m_pPrinterModel->blockSignals(true);
        pItem->setText(m_CurPrinterName);
        m_pPrinterListView->blockSignals(false);
        m_pPrinterModel->blockSignals(false);
        return;
    }
    if (m_pPrinterManager->hasSamePrinter(newPrinterName)) {
        DDialog *pDialog = new DDialog();
        pDialog->setIcon(QIcon(":/images/warning_logo.svg"));
        QLabel *pMessage = new QLabel(tr("The name already exists"));
        pMessage->setWordWrap(true);
        pMessage->setAlignment(Qt::AlignCenter);
        pDialog->addContent(pMessage);
        pDialog->addButton(tr("OK"));
        pDialog->exec();
        pDialog->deleteLater();
        m_pPrinterListView->blockSignals(true);
        m_pPrinterModel->blockSignals(true);
        pItem->setText(m_CurPrinterName);
        m_pPrinterListView->blockSignals(false);
        m_pPrinterModel->blockSignals(false);
        return;
    }

    try {
        if (m_pPrinterManager->hasFinishedJob()) {
            DDialog *pDialog = new DDialog();
            pDialog->setIcon(QIcon(":/images/warning_logo.svg"));
            QLabel *pMessage = new QLabel(tr("You will not be able to reprint the completed jobs if continue. Are you sure you want to rename the printer?"));
            pMessage->setWordWrap(true);
            pMessage->setAlignment(Qt::AlignCenter);
            pDialog->addContent(pMessage);
            pDialog->addButton(UI_PRINTERSHOW_CANCEL);
            int okIndex = pDialog->addButton(tr("Confirm"));

            int ret = pDialog->exec();
            pDialog->deleteLater();
            if (ret != okIndex) {
                //避免重复触发itemchanged信号
                m_pPrinterListView->blockSignals(true);
                m_pPrinterModel->blockSignals(true);
                pItem->setText(m_CurPrinterName);
                m_pPrinterListView->blockSignals(false);
                m_pPrinterModel->blockSignals(false);
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
        refreshPrinterListView(m_CurPrinterName);
    } catch (const std::runtime_error &e) {
        qWarning() << e.what();
    }
}

void DPrintersShowWindow::printSettingClickSlot()
{
    if (!m_pPrinterListView->currentIndex().isValid())
        return ;
    QString strPrinterName = m_pPrinterListView->currentIndex().data().toString();
    DPropertySetDlg dlg(this);
    dlg.setPrinterName(strPrinterName);

    if (dlg.isDriveBroken()) {
        DDialog dialog;
        dialog.setFixedSize(QSize(400, 150));
        dialog.setMessage(tr("The driver is damaged, please install it again."));
        dialog.addSpacing(10);
        dialog.addButton(UI_PRINTERSHOW_CANCEL);
        int iIndex = dialog.addButton(tr("install driver"));
        dialog.setIcon(QPixmap(":/images/warning_logo.svg"));
        QAbstractButton *pBtn = dialog.getButton(iIndex);
        connect(pBtn, &QAbstractButton::clicked, this, &DPrintersShowWindow::printDriveInstall);
        QPoint ptCen1 = this->geometry().center();
        QPoint ptCen2 = dialog.geometry().center();
        QPoint mvPhasor = ptCen1 - ptCen2;
        QRect dialogRc = dialog.geometry();
        dialogRc.moveCenter(dialogRc.center() + mvPhasor);
        dialog.setGeometry(dialogRc);
        dialog.exec();
    } else {
        dlg.updateViews();
        dlg.moveToParentCenter();
        dlg.exec();
    }
}

void DPrintersShowWindow::printQueueClickSlot()
{
    g_printerApplication->showJobsWindow();
}

void DPrintersShowWindow::printTestClickSlot()
{
    if (!m_pPrinterListView->currentIndex().isValid())
        return;
    m_pTBtnPrintTest->blockSignals(true);
    QString printerName = m_pPrinterListView->currentIndex().data().toString();

    PrinterTestPageDialog *dlg = new PrinterTestPageDialog(printerName, this);
    connect(dlg, &PrinterTestPageDialog::signalFinished, this, [ = ]() {
        m_pTBtnPrintTest->blockSignals(false);
    });
    dlg->printTestPage();
}

void DPrintersShowWindow::printFalutClickSlot()
{
    if (!m_pPrinterListView->currentIndex().isValid())
        return;
    QString printerName = m_pPrinterListView->currentIndex().data(Qt::DisplayRole).toString();

    TroubleShootDialog dlg(printerName, this);
    dlg.setModal(true);
    dlg.exec();
}

void DPrintersShowWindow::printDriveInstall()
{
    m_pSearchWindow->show();
}

void DPrintersShowWindow::printerListWidgetItemChangedSlot(const QModelIndex &previous)
{
    Q_UNUSED(previous)
    //在清空QListWidget的时候会触发这个槽函数，需要先判断是否这个item存在，不然会导致段错误
    if (!m_pPrinterListView->currentIndex().isValid())
        return;
    QString printerName = m_pPrinterListView->currentIndex().data(Qt::DisplayRole).toString();
    m_CurPrinterName = printerName;
    QStringList basePrinterInfo = m_pPrinterManager->getPrinterBaseInfoByName(printerName);
    if (basePrinterInfo.count() == 3) {
        QString showPrinter = printerName;
        //如果文字超出了显示范围，那么就用省略号代替，并且设置tip提示
        geteElidedText(m_pLabelPrinterName->font(), showPrinter, 60 + m_pLabelLocationShow->width());
        m_pLabelPrinterName->setText(showPrinter);
        m_pLabelPrinterName->setToolTip(printerName);

        QString location = basePrinterInfo.at(0);
        geteElidedText(m_pLabelLocationShow->font(), location, m_pLabelLocationShow->width());
        m_pLabelLocationShow->setText(location);
        m_pLabelLocationShow->setToolTip(basePrinterInfo.at(0));

        QString model = basePrinterInfo.at(1);
        geteElidedText(m_pLabelTypeShow->font(), model, m_pLabelTypeShow->width());
        m_pLabelTypeShow->setText(model);
        m_pLabelTypeShow->setToolTip(basePrinterInfo.at(1));
        m_pLabelStatusShow->setText(basePrinterInfo.at(2));
    }
}

void DPrintersShowWindow::contextMenuRequested(const QPoint &point)
{
    if (m_pPrinterListView->currentIndex().isValid()) {
        QString printerName = m_pPrinterListView->currentIndex().data().toString();
        bool isShared = m_pPrinterManager->isPrinterShared(printerName);
        m_pShareAction->setChecked(isShared);
        bool isEnabled = m_pPrinterManager->isPrinterEnabled(printerName);
        m_pEnableAction->setChecked(isEnabled);
        bool isAcceptJob = m_pPrinterManager->isPrinterAcceptJob(printerName);
        m_pRejectAction->setChecked(isAcceptJob);
        bool isDefault = m_pPrinterManager->isDefaultPrinter(printerName);
        m_pDefaultAction->setChecked(isDefault);
        // 当打印机为默认时，不让用户取消，避免出现没有默认打印机的状态
        m_pDefaultAction->setDisabled(isDefault);
        m_pListViewMenu->popup(mapToParent(point));
    }
}

void DPrintersShowWindow::listWidgetMenuActionSlot(bool checked)
{
    QString printerName = m_pPrinterListView->currentIndex().data().toString();
    if (sender()->objectName() == "Share") {
        m_pPrinterManager->setPrinterShared(printerName, checked);
    } else if (sender()->objectName() == "Enable") {
        m_pPrinterManager->setPrinterEnabled(printerName, checked);
        //更新打印机状态信息
        printerListWidgetItemChangedSlot(m_pPrinterListView->currentIndex());

    } else if (sender()->objectName() == "Default") {
        if (checked) {
            m_pPrinterManager->setPrinterDefault(printerName);
            updateDefaultPrinterIcon();
        }
    } else if (sender()->objectName() == "Accept") {
        m_pPrinterManager->setPrinterAcceptJob(printerName, checked);
    }
}





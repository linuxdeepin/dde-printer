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
#include "installdriverwindow.h"
#include "zdrivermanager.h"
#include "addprinter.h"
#include "installprinterwindow.h"
#include "cupsattrnames.h"
#include "printerservice.h"
#include "printersearchwindow.h"
#include "uisourcestring.h"
#include "common.h"

#include <DListView>
#include <DLineEdit>
#include <DTitlebar>
#include <DWidgetUtil>
#include <DFileDialog>
#include <DSpinner>

#include <QStandardItemModel>
#include <QStandardItem>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QStackedWidget>
#include <QPushButton>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QCompleter>

InstallDriverWindow::InstallDriverWindow(QWidget *parent)
    : DMainWindow(parent)
{
    initUI();
    initConnections();
}

InstallDriverWindow::~InstallDriverWindow()
{

}

void InstallDriverWindow::initUI()
{
    titlebar()->setMenuVisible(false);
    titlebar()->setTitle("");
    titlebar()->setIcon(QIcon(":/images/dde-printer.svg"));
    m_pPreBtn = new DIconButton(this);
    m_pPreBtn->setIcon(QIcon::fromTheme("dp_arrow_left"));
    titlebar()->addWidget(m_pPreBtn, Qt::AlignLeft);
    // 去掉最大最小按钮
    setWindowFlags(windowFlags() & ~Qt::WindowMinMaxButtonsHint);
    setWindowModality(Qt::ApplicationModal);
    resize(682, 532);
    // 左侧
    QLabel *pLabelTitle1 = new QLabel(tr("Select drive source")) ;
    QFont font;
    font.setBold(true);
    pLabelTitle1->setFont(font);
    m_pTabListView = new DListView();
    m_pTabListModel = new QStandardItemModel();
    m_pTabListModel->appendRow(new QStandardItem(tr("Local driver")));
    m_pTabListModel->appendRow(new QStandardItem(tr("Local PPD file")));
    m_pTabListModel->appendRow(new QStandardItem(tr("Set up the print information search driver")));
    m_pTabListView->setModel(m_pTabListModel);
    m_pTabListView->setCurrentIndex(m_pTabListModel->index(0, 0));
    m_pTabListView->setEditTriggers(QListView::EditTrigger::NoEditTriggers);
    QVBoxLayout *pLeftVBoxlayout = new QVBoxLayout();
    pLeftVBoxlayout->addWidget(pLabelTitle1);
    pLeftVBoxlayout->addWidget(m_pTabListView);

    // 切换控件
    m_pStackWidget = new QStackedWidget();
    m_pRightTitleLabel = new QLabel(tr("Select local driver"));// 这个标题栏三个界面公用
    m_pRightTitleLabel->setFont(font);
    //右侧 本地
    QLabel *pLabelManufacturer = new QLabel(tr("Manufacturer"));
    m_pManufacturerCombo = new QComboBox();
    m_pManufacturerCombo->setMaxVisibleItems(10);
    m_pManufacturerCombo->setEditable(true);
    QLabel *pLabelType = new QLabel(tr("Type"));
    m_pTypeCombo = new QComboBox();
    m_pTypeCombo->setEditable(true);
    QLabel *pLabelDriver = new QLabel(tr("Driver"));
    m_pDriverCombo = new QComboBox();
    m_pDriverCombo->setEditable(true);
    QGridLayout *pGLayout = new QGridLayout();
    pGLayout->addWidget(pLabelManufacturer, 0, 0);
    pGLayout->addWidget(m_pManufacturerCombo, 0, 1);
    pGLayout->addWidget(pLabelType, 1, 0);
    pGLayout->addWidget(m_pTypeCombo, 1, 1);
    pGLayout->addWidget(pLabelDriver, 2, 0);
    pGLayout->addWidget(m_pDriverCombo, 2, 1);
    pGLayout->setColumnStretch(0, 1);
    pGLayout->setColumnStretch(1, 3);
    pGLayout->setRowStretch(3, 1);
    QWidget *pLocalWidget = new QWidget();
    pLocalWidget->setLayout(pGLayout);
    m_pStackWidget->addWidget(pLocalWidget);

    // PPD
    m_pPPDPath = new QLabel(UI_PRINTERDRIVER_PPDLABEL_NORMAL);
    m_pPPDPath->installEventFilter(this);
    m_pPPDPath->setAcceptDrops(true);
    m_pPPDPath->setAlignment(Qt::AlignCenter);
    m_pPPDPath->setWordWrap(true);
    m_pSelectPPDBtn = new QPushButton(UI_PRINTERDRIVER_PPDBTN_NORMAL);
    QVBoxLayout *pVLayout = new QVBoxLayout();
    pVLayout->setContentsMargins(0, 100, 0, 100);
    pVLayout->addWidget(m_pPPDPath, 1, Qt::AlignCenter);
    pVLayout->addWidget(m_pSelectPPDBtn, 1, Qt::AlignCenter);

    QWidget *pPPDWidget = new QWidget();
    pPPDWidget->setObjectName("ppdWidget");

    pPPDWidget->setLayout(pVLayout);
    m_pStackWidget->addWidget(pPPDWidget);

    //设置打印信息搜索
    QLabel *pMakerAndTypeLabel = new QLabel(tr("Manufacturer and model"));
    m_pManuAndTypeLineEdit = new QLineEdit();
    m_pManuAndTypeLineEdit->setValidator(new QRegExpValidator(QRegExp("^[a-zA-Z0-9 ]*$")));
    m_pSearchBtn = new QPushButton(tr("detection"));
    QLabel *pDriverLabel = new QLabel(tr("Driver"));
    m_pDriverManualCombo = new QComboBox();
    QGridLayout *pGLayout1 = new QGridLayout();
    pGLayout1->addWidget(pMakerAndTypeLabel, 0, 0);
    pGLayout1->addWidget(m_pManuAndTypeLineEdit, 0, 1);
    pGLayout1->addWidget(m_pSearchBtn, 0, 2);
    pGLayout1->addWidget(pDriverLabel, 1, 0);
    pGLayout1->addWidget(m_pDriverManualCombo, 1, 1, 1, 2);
    pGLayout1->setRowStretch(2, 1);
    QWidget *pSettingWidget = new QWidget();
    pSettingWidget->setLayout(pGLayout1);
    m_pStackWidget->addWidget(pSettingWidget);

    //安装按钮
    m_pInstallBtn = new QPushButton(tr("Start the installation"));
    m_pSpinner = new DSpinner();
    m_pSpinner->setFixedSize(32, 32);
    // 右侧整体布局
    QVBoxLayout *pRightVLayout = new QVBoxLayout();
    pRightVLayout->addWidget(m_pRightTitleLabel);
    pRightVLayout->addWidget(m_pStackWidget);
    pRightVLayout->addWidget(m_pInstallBtn, 0, Qt::AlignCenter);
    pRightVLayout->addWidget(m_pSpinner, 0, Qt::AlignCenter);
    pRightVLayout->addSpacing(20);

    // 整体布局
    QHBoxLayout *pMainLayout = new QHBoxLayout();
    pMainLayout->addLayout(pLeftVBoxlayout, 1);
    pMainLayout->addLayout(pRightVLayout, 2);
    QWidget *centralWidget = new QWidget;
    centralWidget->setLayout(pMainLayout);
    takeCentralWidget();
    setCentralWidget(centralWidget);

    moveToCenter(this);
}

void InstallDriverWindow::initConnections()
{
    connect(m_pPreBtn, &DIconButton::clicked, this, [this]() {
        close();
        if (m_pParentWidget)
            m_pParentWidget->show();
    });
    connect(m_pTabListView, QOverload<const QModelIndex &>::of(&DListView::currentChanged), this, &InstallDriverWindow::tabCurrentIndexChanged);
    connect(m_pSelectPPDBtn, &QPushButton::clicked, this, &InstallDriverWindow::getPPDFileFromLocalSlot);
    connect(m_pInstallBtn, &QPushButton::clicked, this, &InstallDriverWindow::installDriverSlot);
    connect(m_pManufacturerCombo, &QComboBox::currentTextChanged, this, &InstallDriverWindow::currentMakerChangedSlot);
    connect(m_pTypeCombo, &QComboBox::currentTextChanged, this, &InstallDriverWindow::currentModelChangedSlot);
    connect(m_pSearchBtn, &QPushButton::clicked, this, &InstallDriverWindow::searchDriverSlot);

    connect(m_pManuAndTypeLineEdit, &QLineEdit::editingFinished, this, [ this ]() {
        // 按下enter会触发两次信号，需要过滤掉失去焦点之后的信号 并且判断校验结果
        if (m_pManuAndTypeLineEdit->hasFocus())
            searchDriverSlot();
    });
}

void InstallDriverWindow::initMakerAndType()
{
    QMap<QString, QMap<QString, QString>> *pMakerModelMap = g_driverManager->getMakeModelNames();
    if (pMakerModelMap) {
        QStringList makerList = pMakerModelMap->keys();
        int index = -1;
        makerList.sort(Qt::CaseInsensitive);
        m_pManufacturerCombo->addItems(makerList);
        //优先显示所选打印机对应的驱动
        if (!m_device.strMakeAndModel.isEmpty()) {
            QString strMake, strModel;
            ppdMakeModelSplit(m_device.strMakeAndModel, strMake, strModel);
            strMake = toNormalName(strMake);
            index = makerList.indexOf(strMake);
        }

        //如果没有和打印机匹配的就选择通用驱动
        if (0 > index) {
            QMap<QString, QString> textdriver = g_driverManager->getTextPPD();
            index = makerList.indexOf(textdriver[CUPS_PPD_MAKE]);
        }

        index = index > -1 ? index : 0;
        m_pManufacturerCombo->setCurrentIndex(index);
    }
}

void InstallDriverWindow::clearUserInfo()
{
    m_pPPDPath ->setText(UI_PRINTERDRIVER_PPDLABEL_NORMAL);
    m_pSelectPPDBtn ->setText(UI_PRINTERDRIVER_PPDBTN_NORMAL);

    m_pManuAndTypeLineEdit->clear();
    m_pDriverManualCombo->clear();
}

void InstallDriverWindow::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    if (g_driverManager->getStatus() < TStat_Suc) {
        //提示本地驱动没有初始化完成
        connect(g_driverManager, &DriverManager::signalStatus, this, &InstallDriverWindow::driverReflushSlot);
        m_pInstallBtn->setVisible(false);
        m_pSpinner->setVisible(true);
        m_pSpinner->start();
    } else {
        m_pSpinner->setVisible(false);
        m_pInstallBtn->setVisible(true);
        if (m_pManufacturerCombo->count() == 0)
            initMakerAndType();
    }
    clearUserInfo();
}

bool InstallDriverWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_pPPDPath) {
        if (event->type() == QEvent::DragEnter) {
            QDragEnterEvent *pDragEE = static_cast<QDragEnterEvent *>(event);
            if (pDragEE) {
                if (pDragEE->mimeData()->hasFormat("text/uri-list")) {
                    if (pDragEE->mimeData()->urls().count() == 1) {
                        QString filePath = pDragEE->mimeData()->urls().at(0).toLocalFile();
                        QFileInfo info(filePath);
                        if (info.suffix() == "ppd") {
                            pDragEE->acceptProposedAction();
                            return true;
                        }
                    }
                }
            }
        } else if (event->type() == QEvent::Drop) {
            QDropEvent *pDropE = static_cast<QDropEvent *>(event);
            if (pDropE) {
                if (pDropE->mimeData()->hasFormat("text/uri-list")) {
                    QList<QUrl> urls = pDropE->mimeData()->urls();
                    if (urls.count() == 1) {
                        QString ppdFilePath = urls.at(0).toLocalFile();
                        if (!ppdFilePath.isEmpty()) {
                            m_pPPDPath->setText(ppdFilePath);
                            m_pSelectPPDBtn->setText(tr("Reselect"));
                            m_pInstallBtn->setEnabled(true);
                        } else {
                            m_pInstallBtn->setEnabled(false);
                        }
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void InstallDriverWindow::tabCurrentIndexChanged()
{
    if (m_pTabListView->currentIndex().row() == 0) {
        m_pStackWidget->setCurrentIndex(0);
        m_pRightTitleLabel->setText(tr("Select local driver"));
        if (m_pDriverCombo->count() == 0)
            m_pInstallBtn->setEnabled(false);
        else {
            m_pInstallBtn->setEnabled(true);
        }
    } else if (m_pTabListView->currentIndex().row() == 1) {
        m_pStackWidget->setCurrentIndex(1);
        m_pRightTitleLabel->setText(tr("Select the PPD file"));
        QFileInfo info(m_pPPDPath->text());
        if (info.exists()) {
            m_pInstallBtn->setEnabled(true);
        } else {
            m_pInstallBtn->setEnabled(false);
        }
    } else if (m_pTabListView->currentIndex().row() == 2) {
        m_pStackWidget->setCurrentIndex(2);
        m_pRightTitleLabel->setText(tr("Set printer information"));
        if (m_pDriverManualCombo->count() == 0)
            m_pInstallBtn->setEnabled(false);
        else {
            m_pInstallBtn->setEnabled(true);
        }
    }
}

void InstallDriverWindow::currentMakerChangedSlot(const QString &maker)
{
    QMap<QString, QMap<QString, QString>> *pMakerModelMap = g_driverManager->getMakeModelNames();
    if (pMakerModelMap) {
        m_pTypeCombo->clear();
        // 去掉重复项
        auto modelSet = pMakerModelMap->value(maker).keys().toSet();
        QStringList modelList = modelSet.toList();
        modelList.sort(Qt::CaseInsensitive);
        m_pTypeCombo->addItems(modelList);
        int index = -1;
        //优先选择打印机对应的驱动
        if (!m_device.strMakeAndModel.isEmpty()) {
            QString strMake, strModel;
            ppdMakeModelSplit(m_device.strMakeAndModel, strMake, strModel);
            index = modelList.indexOf(strModel);
        }

        //如果没有匹配的，则选择通用的驱动
        if (0 > index) {
            QMap<QString, QString> textdriver = g_driverManager->getTextPPD();
            index = modelList.indexOf(textdriver[CUPS_PPD_MODEL]);
        }
        index = index > -1 ? index : 0;
        m_pTypeCombo->setCurrentIndex(index);
    }
}

void InstallDriverWindow::currentModelChangedSlot(const QString &model)
{
    QMap<QString, QMap<QString, QString>> *pMakerModelMap = g_driverManager->getMakeModelNames();
    if (pMakerModelMap) {
        m_pDriverCombo->clear();
        QStringList ppdKeys = pMakerModelMap->value(m_pManufacturerCombo->currentText()).values(model);
        QMap<QString, QMap<QString, QString>> *ppds = g_driverManager->getPPDs();
        foreach (QString key, ppdKeys) {
            QList<QMap<QString, QString>> list = ppds->values(key.toLower());
            for (int i = 0; i < list.count(); i++) {
                QString strPpd = list[i].value("ppd-name");
                QString ppdname = list[i].value("ppd-make-and-model");
                if (ppdname.isEmpty())
                    ppdname = strPpd;
                if (ppdname.contains("(recommended)")) {
                    ppdname.remove("(recommended)");
                    ppdname.append(tr("(recommended)"));
                }
                m_pDriverCombo->addItem(ppdname, QVariant::fromValue(list[i]));
            }
        }
    }
    if (m_pDriverCombo->count() > 0) {
        m_pInstallBtn->setEnabled(true);
    } else {
        m_pInstallBtn->setEnabled(false);
    }
}

void InstallDriverWindow::getPPDFileFromLocalSlot()
{
    QString ppdFilePath = DFileDialog::getOpenFileName(this, tr("Select the PPD file"), "/home", "*.ppd");
    if (!ppdFilePath.isEmpty()) {
        m_pPPDPath->setText(ppdFilePath);
        m_pSelectPPDBtn->setText(tr("Reselect"));
        m_pInstallBtn->setEnabled(true);
    } else {
        m_pInstallBtn->setEnabled(false);
    }
}

void InstallDriverWindow::installDriverSlot()
{
    QMap<QString, QVariant> solution;
    QMap<QString, QVariant> dataMap;

    if (m_pTabListView->currentIndex().row() == 0) {
        if (m_pDriverCombo->count() <= 0)
            return;
        solution = m_pDriverCombo->currentData().value<QMap<QString, QVariant>>();
        int driverCount = m_pDriverCombo->count();
        for (int i = 0; i < driverCount; ++i) {
            if (i != m_pDriverCombo->currentIndex()) {
                dataMap.insert(m_pDriverCombo->itemText(i), m_pDriverCombo->itemData(i));
            }
        }
    } else if (m_pTabListView->currentIndex().row() == 1) {
        QFileInfo info(m_pPPDPath->text());
        if (!info.exists()) {
            return;
        }
        solution.insert(SD_KEY_from, PPDFrom_File);
        solution.insert(CUPS_PPD_NAME, m_pPPDPath->text());
    } else if (m_pTabListView->currentIndex().row() == 2) {
        if (m_pDriverManualCombo->count() <= 0)
            return;
        solution = m_pDriverManualCombo->currentData().value<QMap<QString, QVariant>>();
        m_device.strMakeAndModel = m_pManuAndTypeLineEdit->text();
        int driverCount = m_pDriverManualCombo->count();
        for (int i = 0; i < driverCount; ++i) {
            if (i != m_pDriverManualCombo->currentIndex()) {
                dataMap.insert(m_pDriverManualCombo->itemText(i), m_pDriverManualCombo->itemData(i));
            }
        }
    }

    AddPrinterTask *task = g_addPrinterFactoty->createAddPrinterTask(m_device, solution);
    InstallPrinterWindow *pInstallPrinterWindow = new InstallPrinterWindow(this);
    pInstallPrinterWindow->setTask(task);
    pInstallPrinterWindow->setDevice(m_device);

    pInstallPrinterWindow->copyDriverData(dataMap);
    close();
    pInstallPrinterWindow->show();
    connect(task, &AddPrinterTask::signalStatus, pInstallPrinterWindow, &InstallPrinterWindow::receiveInstallationStatusSlot);
    PrinterSearchWindow *pSearch = static_cast<PrinterSearchWindow *>(m_pParentWidget);
    if (pSearch)
        connect(pInstallPrinterWindow, &InstallPrinterWindow::updatePrinterList, pSearch, &PrinterSearchWindow::updatePrinterList);
    task->doWork();
}

void InstallDriverWindow::searchDriverSlot()
{
    if (m_pManuAndTypeLineEdit->text().isEmpty())
        return;
    m_pSearchBtn->blockSignals(true);
    // 拷贝结构体数据，避免修改原始数据
    TDeviceInfo device = m_device;
    device.strMakeAndModel = m_pManuAndTypeLineEdit->text();
    device.strDeviceId.clear();
    DriverSearcher *pDriverSearcher = g_driverManager->createSearcher(device);
    connect(pDriverSearcher, &DriverSearcher::signalDone, this, &InstallDriverWindow::driverSearchedSlot);
    pDriverSearcher->startSearch();
}

void InstallDriverWindow::driverReflushSlot(int id, int iState)
{
    Q_UNUSED(id)
    if (iState == TStat_Suc) {
        m_pSpinner->stop();
        m_pSpinner->setVisible(false);
        m_pInstallBtn->setVisible(true);
        initMakerAndType();
    }
}

void InstallDriverWindow::driverSearchedSlot()
{
    m_pDriverManualCombo->clear();
    DriverSearcher *pDriverSearcher = static_cast<DriverSearcher *>(sender());
    if (pDriverSearcher) {
        QList<QMap<QString, QVariant>> drivers = pDriverSearcher->getDrivers();
        pDriverSearcher->deleteLater();
        foreach (auto driver, drivers) {
            //将驱动结构体存在item中，方便后续安装打印机
            QString strDesc;
            if (driver.value("ppd-make-and-model").isNull())
                strDesc = driver.value("ppd-make").toString() + " " + driver.value("ppd-model").toString();
            else
                strDesc = driver.value("ppd-make-and-model").toString();

            bool isExcat = true;
            QString strReplace;
            //本地都是精确匹配，服务器获取可能不是精确匹配
            if (driver.value(SD_KEY_from).toInt() == PPDFrom_Server) {
                isExcat = driver.value("excat").toBool();
            }
            if (isExcat) strReplace = tr(" (recommended)");
            //去掉自带的recommended字段
            strDesc.replace("(recommended)", "");
            //如果是精确匹配添加 推荐 字段
            strDesc += strReplace;
            m_pDriverManualCombo->addItem(strDesc, QVariant::fromValue(driver));
        }
        if (m_pDriverManualCombo->count() == 0) {
            m_pInstallBtn->setEnabled(false);
        } else {
            m_pInstallBtn->setEnabled(true);
        }
    }
    m_pSearchBtn->blockSignals(false);
}

void InstallDriverWindow::setDeviceInfo(const TDeviceInfo &device)
{
    m_device = device;
}

void InstallDriverWindow::setPreWidget(QWidget *parent)
{
    m_pParentWidget = parent;
}

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

#include "printerapplication.h"
#include "jobmanagerwindow.h"
#include "dprintersshowwindow.h"
#include "zdrivermanager.h"
#include "zcupsmonitor.h"
#include "dprintermanager.h"
#include "zsettings.h"
#include "zjobmanager.h"

#include <DApplication>
#include <DLog>
#include <DWidgetUtil>
#include <DGuiApplicationHelper>
#include <QTranslator>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

enum ApplicationModel{
    APPMODEL_Watch = 0x1,
    APPMODEL_TrayIcon = 0x2,
    APPMODEL_JobsWindow = 0x4,
    APPMODEL_MainWindow = 0x8,
};

PrinterApplication *PrinterApplication::getInstance()
{
    static PrinterApplication *instance = nullptr;
    if (!instance)
        instance = new PrinterApplication();

    return instance;
}

void PrinterApplication::slotNewProcessInstance(qint64 pid, const QStringList &arguments)
{
    Q_UNUSED(pid);

    launchWithMode(arguments);
}

int PrinterApplication::launchWithMode(const QStringList &arguments)
{
    int allModel = APPMODEL_Watch | APPMODEL_TrayIcon | APPMODEL_JobsWindow | APPMODEL_MainWindow;
    int appModel = 0;
    if (arguments.count() > 2 && arguments[1] == "-m") {
        appModel = arguments[2].toInt() & allModel;
    }

    appModel = appModel>0?appModel:APPMODEL_MainWindow;

    if (appModel & APPMODEL_Watch) {
        qApp->setQuitOnLastWindowClosed(false);
    }

    if (appModel & APPMODEL_JobsWindow) {
        showJobsWindow();
    }

    if ((appModel & APPMODEL_MainWindow)) {
        showMainWindow();
    }

    return 0;
}

int PrinterApplication::create()
{
    if (!qApp) return -1;

    qApp->loadTranslator();
    qApp->setAttribute(Qt::AA_UseHighDpiPixmaps);
    qApp->setOrganizationName("deepin");
    qApp->setApplicationName("dde-printer");
    qApp->setApplicationVersion("1.0");
    qApp->setProductIcon(QIcon(":/images/dde-printer.svg"));
    qApp->setProductName(tr("Print Manager"));
    qApp->setApplicationDescription(tr("Print Manager is a printer management tool, which supports adding and removing printers, managing print jobs and so on."));
    qApp->setApplicationLicense("GPLv3");

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();
    QString logRules = g_Settings->getLogRules();
    QLoggingCategory::setFilterRules(logRules);
    qInfo() << "save log to:" << DLogManager::getlogFilePath();

    g_cupsMonitor->initTranslations();
    DPrinterManager::getInstance()->initLanguageTrans();

    if (!DGuiApplicationHelper::setSingleInstance("dde-printer")) {
        qWarning() << "dde-printer is running";
        return -2;
    }

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::newProcessInstance, this, &PrinterApplication::slotNewProcessInstance);
    connect(g_cupsMonitor, &CupsMonitor::signalShowTrayIcon, this, &PrinterApplication::slotShowTrayIcon);

    g_cupsMonitor->initSubscription();
    g_cupsMonitor->initWatcher();

    return 0;
}

int PrinterApplication::stop()
{
    g_driverManager->stop();
    g_cupsMonitor->stop();

    delete DPrinterManager::getInstance();
    return 0;
}

int PrinterApplication::showJobsWindow()
{
    if (!m_jobsWindow) {
        m_jobsWindow = new JobManagerWindow();
        Dtk::Widget::moveToCenter(m_jobsWindow);
    }

    g_cupsMonitor->start();
    m_jobsWindow->showNormal();
    qApp->setActiveWindow(m_jobsWindow);

    return 0;
}

int PrinterApplication::showMainWindow()
{
    if (!m_mainWindow) {
        m_mainWindow = new DPrintersShowWindow();
        Dtk::Widget::moveToCenter(m_mainWindow);

        connect(m_mainWindow, &DPrintersShowWindow::signalMainWindowClosed, this, &PrinterApplication::slotMainWindowClosed);

        // 初始化驱动
        g_driverManager->refreshPpds();
    }

    m_mainWindow->showNormal();
    qApp->setActiveWindow(m_mainWindow);

    return 0;
}

void PrinterApplication::slotMainWindowClosed()
{
    qInfo() << "";
    if (m_mainWindow) {
        m_mainWindow->deleteLater();
        m_mainWindow = nullptr;
        g_driverManager->stop();
    }
}

void PrinterApplication::slotShowTrayIcon(bool bShow)
{
    if (bShow && !m_systemTray) {
        m_systemTray = new QSystemTrayIcon(QIcon(":/images/dde-printer.svg"));
        connect(m_systemTray, &QSystemTrayIcon::activated, [this](QSystemTrayIcon::ActivationReason reason) {
                if (reason == QSystemTrayIcon::Trigger) {
                    showJobsWindow();
                }
                });
    }

    if (bShow)
        m_systemTray->show();
    else if (m_systemTray)
        m_systemTray->hide();
}

PrinterApplication::PrinterApplication()
    : QObject(nullptr),
      m_jobsWindow(nullptr),
      m_mainWindow(nullptr),
      m_systemTray(nullptr)
{}

PrinterApplication::~PrinterApplication()
{
    if (m_jobsWindow) delete m_jobsWindow;

    if (m_mainWindow) delete m_mainWindow;

    if (m_systemTray) delete m_systemTray;
}

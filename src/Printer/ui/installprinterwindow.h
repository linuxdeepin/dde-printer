/*
 * Copyright (C) 2019 ~ 2020 Uniontech Software Co., Ltd.
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
#ifndef INSTALLPRINTERWINDOW_H
#define INSTALLPRINTERWINDOW_H
#include "addprinter.h"

#include <DMainWindow>
#include <DImageButton>

DWIDGET_USE_NAMESPACE
DWIDGET_BEGIN_NAMESPACE
class DIconButton;
class DSpinner;
DWIDGET_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QComboBox;
class QPushButton;
class QWidget;
class QLabel;
class QStackedWidget;
QT_END_NAMESPACE
enum InstallationStatus {
    Installing = 0,
    Installed,
    Printing,
    Printed,
    PrintFailed,
    Reinstall
};

class PrinterTestJob;

class InstallPrinterWindow : public DMainWindow
{
    Q_OBJECT
public:
    explicit InstallPrinterWindow(QWidget *parennt = nullptr);
    virtual ~InstallPrinterWindow() override;

    void setTask(AddPrinterTask *task);
    void setDefaultPrinterName(const QString &name);
    void setDevice(const TDeviceInfo &device);
    void copyDriverData(QComboBox *source) Q_DECL_DEPRECATED;
    /**
    * @projectName   Printer
    * @brief         拷贝上一级界面的备选驱动
    * @author        liurui
    * @date          2019-11-01
    */
    void copyDriverData(const QMap<QString, QVariant> &itemDataMap);

private:
    void initUI();
    void initConnections();

    void setStatus(InstallationStatus status);

    void feedbackPrintTestPage();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // 响应取消按钮
    void cancelBtnClickedSlot();
    // 响应左侧按钮
    void leftBtnClickedSlot();
    // 响应右侧按钮
    void rightBtnClickedSlot();
public slots:
    // 响应后台安装打印机线程的状态消息
    void receiveInstallationStatusSlot(int status);

signals:
    /**
    * @projectName   Printer
    * @brief         通知主界面刷新打印机列表，参数是新
    * 添加的打印机名称，方便后续选中操作
    * @author        liurui
    * @date          2019-11-07
    */
    void updatePrinterList(const QString &newPrinterName);

private:
    DSpinner *m_pSpinner;

    QLabel *m_pStatusImageLabel;
    QLabel *m_pStatusLabel;
    QLabel *m_pTipLabel;
    QPushButton *m_pCancelInstallBtn;

    // 这两个按钮需要在不同的阶段切换文案，实现不同功能，实现复用
    QPushButton *m_pCheckPrinterListBtn;
    QPushButton *m_pPrinterTestPageBtn;

    QComboBox *m_pDriverCombo;

    AddPrinterTask *m_pAddPrinterTask;

private:
    InstallationStatus m_status;
    QString m_printerName;

    TDeviceInfo m_device;
    // 用于安装失败之后，返回上级界面，有两种情况
    QWidget *m_pParentWidget;
    bool m_bInstallFail;

    PrinterTestJob *m_testJob;
};

#endif // INSTALLPRINTERWINDOW_H

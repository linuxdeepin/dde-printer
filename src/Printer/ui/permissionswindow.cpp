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
#include "permissionswindow.h"

#include <DLineEdit>
#include <DPasswordEdit>
#include <DTitlebar>
#include <DWidgetUtil>

#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCloseEvent>
#include <QEventLoop>
PermissionsWindow::PermissionsWindow(QWidget *parent)
    : DMainWindow(parent)
    , m_ret(0)
{
    initUI();
    initConnections();
}

PermissionsWindow::~PermissionsWindow()
{

}

int PermissionsWindow::exec()
{
    show();
    QEventLoop loop;
    connect(this, &PermissionsWindow::finished, &loop, &QEventLoop::quit);
    loop.exec();
    return m_ret;
}


void PermissionsWindow::setHost(const QString &host)
{
    m_host = host;
    m_pTipLabel->setText(tr("To find the printer, log in %1").arg(m_host));
}

QString PermissionsWindow::getUser()
{
    return m_pUserEdit->text();
}

QString PermissionsWindow::getGroup()
{
    return m_pGroupEdit->text();
}

QString PermissionsWindow::getPassword()
{
    return m_pPasswordEdit->text();
}

void PermissionsWindow::initUI()
{
    titlebar()->setMenuVisible(false);
    titlebar()->setTitle(tr(""));
    titlebar()->setIcon(QIcon(":/images/dde-printer.svg"));
    // 去掉最大最小按钮
    setWindowFlags(windowFlags() & ~Qt::WindowMinMaxButtonsHint);
    setWindowModality(Qt::ApplicationModal);
    resize(380, 270);

    m_pTipLabel = new QLabel("");
    m_pUserEdit = new DLineEdit();
    QLabel *pUserLabel = new QLabel(tr("Username"));
    m_pGroupEdit = new DLineEdit();
    QLabel *pGroupLabel = new QLabel(tr("Group"));
    m_pPasswordEdit = new DPasswordEdit();
    QLabel *pPwdLabel = new QLabel(tr("Password"));

    m_pCancelBtn = new QPushButton(tr("Cancel"));
    m_pConnectBtn = new QPushButton(tr("Connect"));
    QBoxLayout *pHLayout = new QHBoxLayout();
    pHLayout->addWidget(m_pCancelBtn);
    pHLayout->addWidget(m_pConnectBtn);

    QGridLayout *pGLayout = new QGridLayout();
    pGLayout->addWidget(m_pTipLabel, 0, 0, 1, 2, Qt::AlignCenter);
    pGLayout->addWidget(pUserLabel, 1, 0, 1, 1);
    pGLayout->addWidget(m_pUserEdit, 1, 1, 1, 1);
    pGLayout->addWidget(pGroupLabel, 2, 0, 1, 1);
    pGLayout->addWidget(m_pGroupEdit, 2, 1, 1, 1);
    pGLayout->addWidget(pPwdLabel, 3, 0, 1, 1);
    pGLayout->addWidget(m_pPasswordEdit, 3, 1, 1, 1);
    QVBoxLayout *pMainLayout = new QVBoxLayout();
    pMainLayout->addLayout(pGLayout);
    pMainLayout->addLayout(pHLayout);

    QWidget *widget = new QWidget();
    widget->setLayout(pMainLayout);
    takeCentralWidget();
    setCentralWidget(widget);

    moveToCenter(this);
}

void PermissionsWindow::initConnections()
{
    connect(m_pCancelBtn, &QPushButton::clicked, this, &PermissionsWindow::btnClickedSlot);
    connect(m_pConnectBtn, &QPushButton::clicked, this, &PermissionsWindow::btnClickedSlot);
}

void PermissionsWindow::btnClickedSlot()
{
    if (sender() == m_pCancelBtn) {
        m_ret = 0;
    } else if (sender() == m_pConnectBtn) {
        m_ret = 1;
    }
    close();
}

void PermissionsWindow::closeEvent(QCloseEvent *event)
{
    emit finished();
    event->accept();
}

void PermissionsWindow::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    m_pUserEdit->clear();
    m_pGroupEdit->clear();
    m_pPasswordEdit->clear();
}

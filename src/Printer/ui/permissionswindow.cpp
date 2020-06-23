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
    : DDialog(parent)
    , m_ret(0)
{
    initUI();
}

PermissionsWindow::~PermissionsWindow()
{
}

void PermissionsWindow::setHost(const QString &host)
{
    m_host = host;
    m_pTipLabel->setText(tr("Connect to %1 to find a printer").arg(m_host));
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
    setIcon(QIcon(":/images/dde-printer.svg"));

    setFixedSize(380, 270);

    m_pTipLabel = new QLabel("");
    m_pUserEdit = new DLineEdit();
    QLabel *pUserLabel = new QLabel(tr("Username"));
    m_pGroupEdit = new DLineEdit();
    QLabel *pGroupLabel = new QLabel(tr("Group"));
    m_pPasswordEdit = new DPasswordEdit();
    QLabel *pPwdLabel = new QLabel(tr("Password"));

    QGridLayout *pGLayout = new QGridLayout();
    pGLayout->addWidget(m_pTipLabel, 0, 0, 1, 2, Qt::AlignCenter);
    pGLayout->addWidget(pUserLabel, 1, 0, 1, 1);
    pGLayout->addWidget(m_pUserEdit, 1, 1, 1, 1);
    pGLayout->addWidget(pGroupLabel, 2, 0, 1, 1);
    pGLayout->addWidget(m_pGroupEdit, 2, 1, 1, 1);
    pGLayout->addWidget(pPwdLabel, 3, 0, 1, 1);
    pGLayout->addWidget(m_pPasswordEdit, 3, 1, 1, 1);

    QWidget *widget = new QWidget();
    widget->setLayout(pGLayout);
    addContent(widget);
    addButton(tr("Cancel"));
    addButton(tr("Connect"), true, ButtonType::ButtonRecommend);

    moveToCenter();
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

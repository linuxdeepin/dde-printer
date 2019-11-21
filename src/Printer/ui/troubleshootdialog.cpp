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

#include "troubleshootdialog.h"
#include "ztroubleshoot.h"
#include "zcupsmonitor.h"
#include "zjobmanager.h"

#include <DPalette>
#include <DApplicationHelper>
#include <DListView>
#include <DWidget>
#include <DFontSizeManager>

#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QAbstractButton>
#include <QPaintEvent>
#include <QPainter>
#include <QFont>

TroubleShootItem::TroubleShootItem(TroubleShootJob* job, int index, QWidget* parent)
    :QFrame (parent),
      m_job(job),
      m_index(index)
{
    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(17, 17);
    m_iconLabel->setPixmap(QIcon(":/images/warning_logo.svg").pixmap(17, 17));
    m_titleLabel = new QLabel(job->getJobName(), this);
    m_titleLabel->setFont(DFontSizeManager::instance()->t5());
    m_messageLabel = new QLabel(this);
    m_messageLabel->setFont(DFontSizeManager::instance()->t7());

    QGridLayout *lay = new QGridLayout(this);
    lay->setContentsMargins(10, 5 ,10, 5);
    lay->setColumnStretch(1, 100);
    lay->addWidget(m_iconLabel, 0, 0, Qt::AlignCenter);
    lay->addWidget(m_titleLabel, 0, 1, Qt::AlignLeft|Qt::AlignHCenter);
    lay->addWidget(m_messageLabel, 1, 1, Qt::AlignLeft|Qt::AlignHCenter);
    setLayout(lay);
    setFixedHeight(60);

    connect(job, &TroubleShootJob::signalStateChanged, this, &TroubleShootItem::slotStateChanged);
}

void TroubleShootItem::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);

    if (0 == (m_index % 2)) {
        DListView listView;
        DPalette pl(DApplicationHelper::instance()->palette(&listView));
        QPainter painter(this);

        painter.fillRect(event->rect(), pl.brush(QPalette::AlternateBase));
    }
}

ContentWidget::ContentWidget(QWidget *parent) : QWidget (parent)
{}

void ContentWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    DListView listView;
    DPalette pl(DApplicationHelper::instance()->palette(&listView));
    QPainter painter(this);

    painter.fillRect(event->rect(), pl.brush(QPalette::Window));
}

void TroubleShootItem::slotStateChanged(int state, const QString &message)
{
    setHidden(false);

    if (TStat_Suc == state)
        m_iconLabel->setPixmap(QIcon(":/images/success.svg").pixmap(17, 17));

    m_messageLabel->setText(message);
}

TroubleShootDialog::TroubleShootDialog(const QString &printerName, QWidget *parent)
    :DDialog (parent)
{
    m_printerName = printerName;

    setIcon(QIcon(":/images/dde-printer.svg"));

    m_trobleShoot = new TroubleShoot(printerName, this);

    QWidget* contentWidget = new ContentWidget(this);
    QLabel* title = new QLabel(tr("Troubleshoot: ") + printerName, contentWidget);
    QFont titleFont = DFontSizeManager::instance()->t5();
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setFixedHeight(30);
    QVBoxLayout* lay = new QVBoxLayout(contentWidget);
    lay->addWidget(title);

    QList<TroubleShootJob*> jobs = m_trobleShoot->getJobs();
    for (int i=0;i<jobs.count();i++) {
        TroubleShootItem *item = new TroubleShootItem(jobs[i], i, this);
        item->hide();
        lay->addWidget(item);
    }
    lay->addStretch(100);
    contentWidget->setFixedSize(692, 432);
    contentWidget->setLayout(lay);
    addContent(contentWidget);

    addButton(tr("cancel"), true);
    getButton(0)->setFixedWidth(200);

    connect(m_trobleShoot, &TroubleShoot::signalStatus, this, &TroubleShootDialog::slotTroubleShootStatus);
    m_trobleShoot->start();
}

void TroubleShootDialog::slotTroubleShootStatus(int id, int state)
{
    Q_UNUSED(id);

    if (TStat_Suc <= state) {
        setButtonText(0, tr("OK"));
    }
}

/*
 * Copyright (C) 2019 ~ 2019 Uniontech Software Co., Ltd.
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
#include "ztroubleshoot_p.h"

#include <DPalette>
#include <DApplicationHelper>
#include <DFrame>
#include <DWidget>
#include <DListView>
#include <DMainWindow>
#include <DFontSizeManager>
#include <DTitlebar>

#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QPaintEvent>
#include <QPainter>
#include <QFont>

TroubleShootItem::TroubleShootItem(TroubleShootJob *job, int index, QWidget *parent)
    : QFrame(parent)
    , m_job(job)
    , m_index(index)
{
    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(17, 17);
    m_iconLabel->setPixmap(QIcon(":/images/warning_logo.svg").pixmap(17, 17));
    m_titleLabel = new QLabel(job->getJobName(), this);
    m_titleLabel->setFont(DFontSizeManager::instance()->t5());
    m_messageLabel = new QLabel(this);
    m_messageLabel->setFont(DFontSizeManager::instance()->t7());

    QGridLayout *lay = new QGridLayout(this);
    lay->setContentsMargins(10, 5, 10, 5);
    lay->setColumnStretch(1, 100);
    lay->addWidget(m_iconLabel, 0, 0, Qt::AlignCenter);
    lay->addWidget(m_titleLabel, 0, 1, Qt::AlignLeft | Qt::AlignHCenter);
    lay->addWidget(m_messageLabel, 1, 1, Qt::AlignLeft | Qt::AlignHCenter);
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
        painter.setBrush(pl.brush(QPalette::AlternateBase));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(event->rect(), 10, 10);
    }
}

void TroubleShootItem::slotStateChanged(int state, const QString &message)
{
    setHidden(false);

    if (TStat_Suc == state)
        m_iconLabel->setPixmap(QIcon(":/images/success.svg").pixmap(17, 17));

    m_messageLabel->setText(message);
}

TroubleShootDialog::TroubleShootDialog(const QString &printerName, QWidget *parent)
    : DAbstractDialog(false, parent)
{
    m_printerName = printerName;

    DTitlebar *titleBar = new DTitlebar();
    titleBar->setIcon(QIcon(":/images/dde-printer.svg"));
    titleBar->setMenuVisible(false);
    titleBar->setTitle("");

    m_trobleShoot = new TroubleShoot(printerName, this);
    m_trobleShoot->addJob(new CheckConnected(printerName, m_trobleShoot));

    setAttribute(Qt::WA_TranslucentBackground, false);
    QWidget *contentWidget = new QWidget(this);

    QLabel *title = new QLabel(tr("Troubleshoot: "), contentWidget);
    title->setFixedHeight(30);
    DFontSizeManager::instance()->bind(title, DFontSizeManager::T5, QFont::DemiBold);
    QHBoxLayout *pHLayout = new QHBoxLayout();
    pHLayout->setContentsMargins(0, 0, 0, 0);
    pHLayout->setSpacing(0);
    pHLayout->addSpacing(10);
    pHLayout->addWidget(title);
    DFrame *frame = new DFrame(contentWidget);
    QVBoxLayout *itemlay = new QVBoxLayout(frame);
    itemlay->setContentsMargins(10, 10, 10, 10);
    itemlay->setSpacing(0);
    itemlay->addLayout(pHLayout);
    itemlay->addSpacing(10);
    QList<TroubleShootJob *> jobs = m_trobleShoot->getJobs();
    for (int i = 0; i < jobs.count(); i++) {
        TroubleShootItem *item = new TroubleShootItem(jobs[i], i, this);
        item->hide();
        itemlay->addWidget(item);
    }
    itemlay->addStretch(100);
    frame->setLayout(itemlay);

    m_button = new QPushButton(contentWidget);
    m_button->setFixedWidth(200);
    m_button->setText(tr("Cancel"));
    m_button->setFocusPolicy(Qt::NoFocus);
    QVBoxLayout *lay = new QVBoxLayout(contentWidget);
    lay->setSpacing(0);
    lay->setContentsMargins(10, 10, 10, 10);
    lay->addWidget(frame, 100);
    lay->addSpacing(10);
    lay->addWidget(m_button, 0, Qt::AlignCenter);

    contentWidget->setFixedSize(692, 432);
    contentWidget->setLayout(lay);

    QVBoxLayout *mainlay = new QVBoxLayout(this);
    mainlay->setContentsMargins(0, 0, 0, 0);
    mainlay->addWidget(titleBar);
    mainlay->addWidget(contentWidget);
    setLayout(mainlay);

    connect(m_trobleShoot, &TroubleShoot::signalStatus, this, &TroubleShootDialog::slotTroubleShootStatus);
    connect(m_button, &QPushButton::clicked, this, &DDialog::close);

    m_trobleShoot->start();
}

void TroubleShootDialog::slotTroubleShootStatus(int id, int state)
{
    Q_UNUSED(id);

    if (TStat_Suc <= state) {
        m_button->setText(tr("OK"));
    }
}

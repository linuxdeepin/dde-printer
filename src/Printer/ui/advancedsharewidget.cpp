// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "advancedsharewidget.h"

#include <DGuiApplicationHelper>
#include <DFontSizeManager>
#include <DApplicationHelper>

#include <QHBoxLayout>
#include <QPainter>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

AdvanceShareWidget::AdvanceShareWidget(QWidget *parent)
    : QWidget(parent)
    , m_textLabel(new QLabel)
    , m_enterIcon(new QLabel)
{
    setMinimumHeight(36);
    updateIcon();
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &AdvanceShareWidget::updateIcon);
    m_enterIcon->setFixedSize(16, 16);
    m_textLabel->setText(tr("Advanced sharing options"));
    m_textLabel->setContentsMargins(2, 0, 0, 0);
    DFontSizeManager::instance()->bind(m_textLabel, DFontSizeManager::T6, QFont::Medium);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_textLabel);
    mainLayout->addWidget(m_enterIcon);
    setLayout(mainLayout);
}

void AdvanceShareWidget::click()
{
    emit clicked();
}

void AdvanceShareWidget::enterEvent(QEvent *event)
{
    QWidget::enterEvent(event);
    m_hover = true;
    update();
}

void AdvanceShareWidget::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    m_hover = false;
    update();
}

void AdvanceShareWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
    emit clicked();
}

void AdvanceShareWidget::updateIcon()
{
    if (DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType()) {
        m_enterIcon->setPixmap(QPixmap(":/images/enter_details_normal.svg"));
    } else {
        m_enterIcon->setPixmap(QPixmap(":/images/enter_details_normal-dark.svg"));
    }

    update();
}

void AdvanceShareWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    const DPalette &dp = DApplicationHelper::instance()->palette(this);
    QPainter p(this);
    p.setPen(Qt::NoPen);
    p.setBrush(dp.brush(DPalette::ItemBackground));
    p.drawRoundedRect(rect(), 8, 8);
    QWidget::paintEvent(event);
}

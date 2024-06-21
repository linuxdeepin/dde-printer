// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ADVANCEDSHAREWIDGET_H
#define ADVANCEDSHAREWIDGET_H

#include <QWidget>
#include <QLabel>

class AdvanceShareWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AdvanceShareWidget(QWidget *parent = nullptr);
    void click();

signals:
    void clicked();

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

public slots:
    void updateIcon();

private:
    QLabel *m_textLabel;
    QLabel *m_enterIcon;
    bool m_hover = false;
};

#endif

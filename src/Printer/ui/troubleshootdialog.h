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

#include <DDialog>
#include <QFrame>

DWIDGET_USE_NAMESPACE

class TroubleShootJob;
class TroubleShoot;

QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
QT_END_NAMESPACE

class TroubleShootItem : public QFrame
{
    Q_OBJECT

public:
    TroubleShootItem(TroubleShootJob *job, int index, QWidget *parent = nullptr);

protected:
    void slotStateChanged(int state, const QString &message);

    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    TroubleShootJob *m_job;
    QLabel *m_iconLabel;
    QLabel *m_titleLabel;
    QLabel *m_messageLabel;
    int m_index;
};

class TroubleShootDialog : public DAbstractDialog
{
    Q_OBJECT

public:
    TroubleShootDialog(const QString &printerName, QWidget *parent = nullptr);

protected slots:
    void slotTroubleShootStatus(int id, int state);

private:
    QString m_printerName;

    TroubleShoot *m_trobleShoot;
    QPushButton *m_button;
};

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

DWIDGET_USE_NAMESPACE

class PrinterTestJob;
class TroubleShoot;

QT_BEGIN_NAMESPACE
class QLabel;
QT_END_NAMESPACE

class PrinterTestPageDialog : public QObject
{
    Q_OBJECT

public:
    PrinterTestPageDialog(const QString &printerName, QWidget *parent = nullptr);

    void printTestPage();

protected slots:
    void slotTroubleShootMessage(int proccess, QString messge);
    void slotTroubleShootStatus(int id, int state);

protected:
    void showErrorMessage(const QString &message);

signals:
    void signalFinished();

private:
    QString m_printerName;

    PrinterTestJob *m_testJob;
    TroubleShoot *m_trobleShoot;
    QString m_message;
    QWidget *m_parent;
};

/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     shenfusheng_cm <shenfusheng_cm@deepin.com>
 *
 * Maintainer: shenfusheng_cm <shenfusheng_cm@deepin.com>
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

#ifndef DPRINTERTANSLATOR_H
#define DPRINTERTANSLATOR_H
#include <QObject>
#include <QMap>

class DPrinterTanslator : public QObject
{
    Q_OBJECT
public:
    DPrinterTanslator();

public:
    void init();
    void addTranslate(const QString &, const QString &, const QString &);
    QString translateLocal(const QString &, const QString &, const QString &);

private:
    QMap<QString, QMap<QString, QString>> m_mapTrans;
};

#endif // DPRINTERTANSLATOR_H

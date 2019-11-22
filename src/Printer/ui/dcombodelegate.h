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

#ifndef DCOMBODELEGATE_H
#define DCOMBODELEGATE_H

#include <QItemDelegate>
#include <QPainter>
#include <QStyledItemDelegate>

enum COMBOITEMROLE
{
    VALUEROLE = Qt::UserRole + 1,
    BACKCOLORROLE
};

enum ITEMSTATE
{
    NORMAL,
    CONFLICT
};

/*
class DComboDelegate:public QItemDelegate
{
public:
    DComboDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,const QModelIndex &index) const override;
};
*/

/*
class ItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT

signals:
    void deleteItem(const QModelIndex &index);

public:
    ItemDelegate(QObject * parent= nullptr);
    ~ItemDelegate(){}
    void paint(QPainter * painter,const QStyleOptionViewItem & option,const QModelIndex & index) const;
};
*/

class ComItemDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    explicit ComItemDelegate(QObject* parent=nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
};

#endif // DCOMBODELEGATE_H

/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     shenfusheng <shenfusheng_cm@deepin.com>
 *
 * Maintainer: shenfusheng <shenfusheng_cm@deepin.com>
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
#ifndef DPRINTERSUPPLYSHOWDLG_H
#define DPRINTERSUPPLYSHOWDLG_H

#include <cupssnmp.h>

#include <DDialog>
#include <DPushButton>

#include <QStackedWidget>
#include <QMap>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class DPrinterSupplyShowDlg : public DDialog
{
    Q_OBJECT
public:
    DPrinterSupplyShowDlg(const QString& strPrinterName, QWidget *parent = nullptr);
    ~DPrinterSupplyShowDlg();

public:
    bool isDriveBroken();
    bool canGetSupplyMsg();
    void updateUI();

private:
    void initUI();
    void initConnection();
    void moveToParentCenter();
    QWidget* initColorSupplyItem(const SUPPLYSDATA& info, bool bColor);
    bool isColorPrinter();
    void initColorTrans();
    QString getTranslatedColor(const QString& strColor);

private:
    QString m_strPrinterName;
    QStackedWidget* m_pStackedWidget;
    QVector<SUPPLYSDATA> m_supplyInfos;
    DPushButton* m_pConfirmBtn;
    QMap<QString,QString> m_mapColorTrans;
};

#endif // DPRINTERSUPPLYSHOWDLG_H

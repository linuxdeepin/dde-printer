/*
 * Copyright (C) 2019 ~ 2019 Uniontech Software Co., Ltd.
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

#ifndef QPROPERTYSETDLG_H
#define QPROPERTYSETDLG_H
//
#include <DSettingsDialog>
#include <DListView>
#include <QMap>
#include "dprinter.h"

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class DPropertySetDlg : public DSettingsDialog
{
    Q_OBJECT
public:
    DPropertySetDlg(const QString &strPrinter = QString(""), QWidget *pParent = nullptr);

public:
    void setPrinterName(const QString &strName);
    void updateViews();
    void moveToParentCenter();
    bool isDriveBroken();
    void initUI();
    void initConnection();

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    void updateComboByName(const QString &, const QString &, const QVector<QMap<QString, QString>> &);
    void updateDriverUI(const QString &);
    void updateUriUI(const QString &);
    void updateLocationUI(const QString &);
    void updateDescriptionUI(const QString &);
    void updatePageSizeCombo(const QString &, const QVector<QMap<QString, QString>> &);
    void updateOrientationCombo(const QString &, const QVector<QMap<QString, QString>> &);
    void updateDuplexPrintCombo(const QString &, const QVector<QMap<QString, QString>> &);
    void updateColorModeCombo(const QString &, const QVector<QMap<QString, QString>> &);
    void updatePrintOrderCombo(const QString &, const QVector<QMap<QString, QString>> &);
    void updatePaperOriginCombo(const QString &, const QVector<QMap<QString, QString>> &);
    void updatePaperTypeCombo(const QString &, const QVector<QMap<QString, QString>> &);
    void updateOutputQuanlityCombo(const QString &, const QVector<QMap<QString, QString>> &);
    void updateStapleLocationCombo(const QString &, const QVector<QMap<QString, QString>> &);
    void updateBindEdgeCombo(const QString &, const QVector<QMap<QString, QString>> &);
    void updateResolutionCombo(const QString &, const QVector<QMap<QString, QString>> &);
    void updateInstallAbleNodeCombo(const QVector<INSTALLABLEOPTNODE> &);
    void updateGeneralNodeCombo(const QVector<GENERALOPTNODE> &);
    QString changeComboSelectionByName(const QString &strComboName, int index);
    bool haveConflict(const QString &, const QString &, QVector<CONFLICTNODE> &);
    bool isConflict(const QString &, const QString &);
    void initConflictMap(const QVector<INSTALLABLEOPTNODE> &, const QVector<GENERALOPTNODE> &);
    void showConflictDlg(const vector<CONFLICTPAIR> &);
    void updateComboByConflits(const QSet<QString> &);
    void checkAllConflicts(QSet<QString> &, vector<CONFLICTPAIR> &);
    QVector<INSTALLABLEOPTNODE> getInstallableNodes();
    QVector<GENERALOPTNODE> getGeneralNodes();

private slots:
    void pageSizeCombo_clicked(int iIndex);
    void paperOrientation_clicked(int iIndex);
    void duplexPrintCombo_clicked(int iIndex);
    void colorModeCombo_clicked(int iIndex);
    void printOrderCombo_clicked(int iIndex);
    void paperOriginCombo_clicked(int iIndex);
    void paperTypeCombo_clicked(int iIndex);
    void printStapleLocationCombo_clicked(int index);
    void printBindEdgeCombo_clicked(int index);
    void printResolutionCombo_clicked(int index);
    void outputQuanlity_clicked(int index);
    void installAbleCombo_clicked(int index);
    void generalCombo_clicked(int index);
    void printUriUI_EditFinished();
    void printLocationUI_EditFinished();
    void printDescriptionUI_EditFinished();
    void confirmBtn_clicked();
    void cancelBtn_clicked();

private:
    QMap<QString, QWidget *> m_mapOfListWidget;
    QMap<QString, QString> m_mapInitUIValue;
    QMap<QString, QString> m_mapDynamicUIValue;
    QMap<QString, QString> m_mapOfConflict;
    QSet<QString> m_setConflictOptions;
    QString m_strPrinterName;
    bool m_bShow;
};

#endif // QPROPERTYSETDLG_H

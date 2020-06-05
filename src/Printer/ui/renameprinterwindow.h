/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     liurui <liurui_cm@deepin.com>
 *
 * Maintainer: liurui <liurui_cm@deepin.com>
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
#ifndef RENAMEPRINTERWINDOW_H
#define RENAMEPRINTERWINDOW_H
#include <dtkwidget_global.h>
#include <DMainWindow>
#include <DPushButton>
DWIDGET_USE_NAMESPACE
DWIDGET_BEGIN_NAMESPACE
class DMainWindow;
class DLineEdit;
DWIDGET_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QPushButton;
QT_END_NAMESPACE

class RenamePrinterWindow : public DMainWindow
{
    Q_OBJECT
public:
    explicit RenamePrinterWindow(QWidget *parent = nullptr);
    virtual ~RenamePrinterWindow() override;

    int result();
    QString getName();
    void setName(const QString &name);
    QString getLocation();
    void setLocation(const QString &location);
    QString getDescription();
    void setDescription(const QString &description);

private:
    void initUI();
    void initConnections();

private slots:
    void btnClickedSlot();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    DLineEdit *m_pNameEdit;
    DLineEdit *m_pLocationEdit;
    DLineEdit *m_pDescriptionEdit;

    DPushButton *m_pCancelBtn;
    DPushButton *m_pOKBtn;

private:
    int m_ret;
signals:
    void accepted(int ret);
};

#endif // RENAMEPRINTERWINDOW_H

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
#ifndef PERMISSIONSWINDOW_H
#define PERMISSIONSWINDOW_H
#include <DDialog>

DWIDGET_USE_NAMESPACE
DWIDGET_BEGIN_NAMESPACE
class DLineEdit;
class DPasswordEdit;
DWIDGET_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
QT_END_NAMESPACE
class PermissionsWindow : public DDialog
{
    Q_OBJECT
public:
    explicit PermissionsWindow(QWidget *parent = nullptr);
    virtual ~PermissionsWindow() override;
    void setHost(const QString &host);
    QString getUser();
    QString getGroup();
    QString getPassword();

private:
    void initUI();


protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    QLabel *m_pTipLabel;
    DLineEdit *m_pUserEdit;
    DLineEdit *m_pGroupEdit;
    DPasswordEdit *m_pPasswordEdit;

private:
    QString m_host;
    QString m_user;
    QString m_group;
    QString m_password;
    int m_ret;
signals:
    void finished();
};

#endif // PERMISSIONSWINDOW_H

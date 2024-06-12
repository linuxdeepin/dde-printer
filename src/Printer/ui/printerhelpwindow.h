// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PRINTERHELPWINDOW_H
#define PRINTERHELPWINDOW_H

#include <QLabel>
#include <DMainWindow>

DWIDGET_USE_NAMESPACE

QT_BEGIN_NAMESPACE
class QWidget;
class QPushButton;
QT_END_NAMESPACE

class PrinterHelpWindow : public DMainWindow
{
    Q_OBJECT
public:
    explicit PrinterHelpWindow(QWidget *parent = nullptr);
    virtual ~PrinterHelpWindow() override;

private:
    void initUi();
    void initConnections();

private slots:

signals:

private:
    QPushButton *m_copyButton;
    QLabel *m_copyLabel;
};

class CustomLabel : public QLabel
{
    Q_OBJECT

public:
    CustomLabel(QWidget *parent = nullptr) : QLabel(parent)
    {
       initUi();
       initConnection();
    }

    void initUi();
    void initConnection();

signals:
    void signalExpand();

protected slots:
    void slotOpenFaqDoc();
    void slotOpenHelpInfo();

private:
    void initSubUi();

protected:
    void changeEvent(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QMenu *m_main;
    QLabel *m_textLabel;
    QPushButton *m_popupButton1;
    QPushButton *m_popupButton2;
    PrinterHelpWindow *m_pHelpWindow = nullptr;
};

#endif // PRINTERHELPWINDOW_H

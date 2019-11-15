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

#ifndef ZLOCALPPDMANAGER_H
#define ZLOCALPPDMANAGER_H

#include "zdevicemanager.h"

#include <QList>
#include <QMap>
#include <QVariant>

enum{
    PPDFrom_Database = 0,
    PPDFrom_File,
    PPDFrom_Server,
    PPDFrom_EveryWhere
};

class ReflushLocalPPDS;

class DriverSearcher : public QObject
{
    Q_OBJECT

public:
    DriverSearcher(const TDeviceInfo &printer, QObject *parent=nullptr);

    void startSearch();

    QList<QMap<QString, QVariant>> getDrivers();

    TDeviceInfo getPrinter();

signals:
    void signalDone();

protected slots:
    void slotDone(int iCode, const QByteArray &result);
    void slotDriverInit(int id, int state);

private:
    int getLocalDrivers();
    void sortDrivers();
    void askForFinish();

    TDeviceInfo     m_printer;
    QList<QMap<QString, QVariant>>   m_drivers;
    int             m_localIndex;

    QString         m_strMake;
    QString         m_strModel;
    QString         m_strCMD;
};

class DriverManager : public QObject
{
    Q_OBJECT

public:
    static DriverManager* getInstance();
    /*!
    * @brief 获取刷新ppd的状态
    */
    int getStatus();

    int stop();

    /*!
    * @brief 刷新ppd列表
    *       非阻塞，开启新的线程，getStatus可以获取当前状态
    */
    int reflushPpds();

    /*!
    * @brief 获取厂商和型号和名字，用来显示给用户选择
    */
    QMap<QString, QMap<QString, QString>>* getMakeModelNames();

    /*!
    * @brief 获取所有ppd文件的信息
    */
    QMap<QString, QMap<QString, QString>>* getPPDs();

    /*!
    * @brief 获取Generic Text-Only Printer，作为默认驱动
    */
    QMap<QString, QString> getTextPPD();

    /*!
    * @brief 判断两个PPD文件路径是否为同一个
    */
    bool isSamePPD(const QString &ppd1, const QString &ppd2);

    QStringList getDriverDepends(const char* strPPD);

    QMap<QString, QVariant> getEveryWhereDriver(const QString &strUri);

    /*!
    * @brief 查找驱动
    */
    DriverSearcher* createSearcher(const TDeviceInfo &device);

signals:
    void signalStatus(int, int);

protected:
    DriverManager(QObject *parent=nullptr);

private:
    ReflushLocalPPDS*   m_reflushTask;
};

#define g_driverManager DriverManager::getInstance()

#endif // ZLOCALPPDMANAGER_H

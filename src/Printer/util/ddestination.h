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
#ifndef DDESTINATION_H
#define DDESTINATION_H
#include "cupsconnection.h"

#include <QStringList>
#include <QObject>

using namespace std;

enum DESTTYPE {
    PRINTER,
    CLASS
};

class DDestination : public QObject
{
    Q_OBJECT
public:
    DDestination(Connection *con);

public:
    QString getName();
    void setName(const QString &strName);

    bool isShared();
    void setShared(bool shared);

    bool isEnabled();
    void setEnabled(bool enabled);

    //BaseInfo
    QStringList getPrinterBaseInfo();
    DESTTYPE getType() const;

    QString printerInfo();
    void setPrinterInfo(const QString &info);

    QString printerLocation();
    void setPrinterLocation(const QString &location);

    QString printerModel();

    virtual bool initPrinterPPD();
    virtual bool isPpdFileBroken();

    /**
    * @projectName   Printer
    * @brief         复制信息用于创建新的打印机
    * @author        liurui
    * @date          2019-11-07
    */
    void getPrinterInfo(QString &info, QString &location, QString &deviceURI, QString &ppdfile) const;

private:
    // 初始化打印机的属性，避免后续重复查询
    void initPrinterAttr();

protected:
    DESTTYPE m_type;
    QString m_strName;
    Connection *m_pCon;

    bool m_isShared;
    bool m_isEnabled;
    QString m_printerLocation;
    QString m_printerStatus;
    QString m_printerInfo; //打印机描述,和型号共用
    QString m_printerURI;
    QString m_strPrinterModel;
    QString m_ppdFile;
};

#endif // DDESTINATION_H

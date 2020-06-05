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

#ifndef SERVICE_H
#define SERVICE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QJsonObject>

#define SD_KEY_from "from"
#define SD_KEY_make_model "describe"
#define SD_KEY_driver "driver"
#define SD_KEY_excat "excat"
#define SD_KEY_ppd "ppd"
#define SD_KEY_sid "sid"
#define SD_KEY_ver "version"
#define SD_KEY_code "client_code"
#define SD_KEY_mfg "MFG"
#define SD_KEY_mdl "MDL"
#define SD_KEY_ieeeid "ieee1284_id"
#define SD_KEY_suc "success"
#define SD_KEY_rid "id"
#define SD_KEY_reason "reason"
#define SD_KEY_feedback "feedback"
#define SD_KEY_detail "detail"
#define SD_KEY_package "package"

class PrinterServerInterface : public QObject
{
    Q_OBJECT

public:
    void postToServer();

signals:
    void signalDone(int, QByteArray);

protected:
    PrinterServerInterface(const QString &url, const QJsonObject &obj, QObject *parent = nullptr);

private:
    QNetworkReply *post_request(const QString &path, const QJsonObject &obj);
    void encrypt(const QString &text, QJsonArray &res);

    QString m_url;
    QJsonObject m_args;

    friend class PrinterService;
};

class PrinterService : public QObject
{
    Q_OBJECT

public:
    static PrinterService *getInstance();

    bool isInvaild();

    PrinterServerInterface *searchSolution(const QString &manufacturer, const QString &model,
                                           const QString &ieee1284_id = "");

    PrinterServerInterface *searchDriver(int solution_id);

    PrinterServerInterface *feedbackResult(int solution_id, bool success,
                                           const QString &reason = "", const QString &feedback = "", int record_id = 0);

protected:
    PrinterService();

private:
    QString m_osVersion;
    QString m_hostname;
    unsigned short m_port;
    QString m_urlPrefix;
    QString m_version;
    QString m_code;
};

#define g_printerServer PrinterService::getInstance()

#endif

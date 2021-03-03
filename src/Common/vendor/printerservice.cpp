/*
 * Copyright (C) 2019 ~ 2020 Uniontech Software Co., Ltd.
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

#include "printerservice.h"
#include "common.h"
#include "config.h"
#include "zsettings.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTimer>
#include <QEventLoop>


static QNetworkAccessManager g_networkManager;

PrinterServerInterface::PrinterServerInterface(const QString &url, const QJsonObject &obj, QObject *parent)
    : QObject(parent)
{
    m_url = url;
    m_args = obj;
}

void PrinterServerInterface::postToServer()
{
    QNetworkReply *reply = post_request(m_url, m_args);

    /*QNetworkReply默认超时时间太长，这里暂定设置为10s*/
    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start(10000);
    loop.exec();

    if (timer.isActive()) {
        /*正常返回*/
        timer.stop();
    } else {
        /*timeout*/
        disconnect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        qWarning() << "timeout(10s)";
    }
    if (reply->error() != QNetworkReply::NoError)
        qWarning() << reply->error();
    QByteArray data = "";
    if (reply->isOpen())
        data = reply->readAll();
    emit signalDone(reply->error(), data);
    reply->deleteLater();
}

QNetworkReply *PrinterServerInterface::post_request(const QString &path, const QJsonObject &obj)
{
    QUrl url = path;
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = g_networkManager.post(request,
                                                 QJsonDocument(obj).toJson(QJsonDocument::Compact));
    return reply;
}

PrinterService *PrinterService::getInstance()
{
    static PrinterService *instance = nullptr;
    if (nullptr == instance)
        instance = new PrinterService();

    return instance;
}

bool PrinterService::isInvaild()
{
    return m_osVersion.split("-").count() != 2;
}

PrinterService::PrinterService()
{
    m_osVersion = g_Settings->getOSVersion();
    if (isInvaild())
        return;

    m_hostname = g_Settings->getHostName();
    m_port = g_Settings->getHostPort();
    m_version = g_Settings->getClientVersion();
    m_code = g_Settings->getClientCode();

    QStringList osargs = m_osVersion.split("-");
    m_urlPrefix = QString("http://%1:%2/%3/%4").arg(m_hostname).arg(m_port).arg(osargs[0]).arg(osargs[1]);
}

PrinterServerInterface *PrinterService::searchSolution(const QString &manufacturer,
                                                       const QString &model, const QString &ieee1284_id)
{
    QJsonObject obj = {
        QPair<QString, QJsonValue>(SD_KEY_ver, m_version),
        QPair<QString, QJsonValue>(SD_KEY_code, m_code),
        QPair<QString, QJsonValue>(SD_KEY_mfg, manufacturer),
        QPair<QString, QJsonValue>(SD_KEY_mdl, model),
        QPair<QString, QJsonValue>(SD_KEY_ieeeid, ieee1284_id),
    };

    if (isInvaild())
        return nullptr;

    qDebug() << "search solution for " << manufacturer << " " << model << " " << ieee1284_id;
    PrinterServerInterface *reply = new PrinterServerInterface(m_urlPrefix + "/search", obj);

    return reply;
}

PrinterServerInterface *PrinterService::searchDriver(int solution_id)
{
    QJsonObject obj = {
        QPair<QString, QJsonValue>(SD_KEY_sid, solution_id),
    };

    if (isInvaild())
        return nullptr;

    PrinterServerInterface *reply = new PrinterServerInterface(m_urlPrefix + "/driver", obj);

    return reply;
}

PrinterServerInterface *PrinterService::feedbackResult(int solution_id, bool success,
                                                       const QString &reason, const QString &feedback, int record_id)
{
    QJsonObject obj = {
        QPair<QString, QJsonValue>(SD_KEY_sid, solution_id),
        QPair<QString, QJsonValue>(SD_KEY_suc, success),
    };
    if (record_id)
        obj.insert(SD_KEY_rid, record_id);

    QJsonObject detail;
    if (!reason.isEmpty())
        detail.insert(SD_KEY_reason, reason);
    if (!feedback.isEmpty())
        detail.insert(SD_KEY_feedback, feedback);
    if (!detail.isEmpty())
        obj.insert(SD_KEY_detail, detail);

    if (isInvaild())
        return nullptr;

    PrinterServerInterface *reply = new PrinterServerInterface(m_urlPrefix + "/report", obj);

    return reply;
}

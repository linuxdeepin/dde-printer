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
#include <cryptopp/base64.h>
#include <cryptopp/osrng.h>
#include <cryptopp/randpool.h>
#include <cryptopp/rsa.h>

using namespace CryptoPP;

static QNetworkAccessManager  g_networkManager;

PrinterServerInterface::PrinterServerInterface(const QString &url, const QJsonObject &obj, QObject* parent)
    :QObject(parent)
{
    m_url = url;
    m_args = obj;
}

void PrinterServerInterface::postToServer()
{
    QNetworkReply *reply = post_request(m_url, m_args);
    connect(reply, &QNetworkReply::finished, [=]() {
        qInfo() << m_url << "return " << reply->error();

        emit signalDone(reply->error(), reply->readAll());
    });
}

QNetworkReply* PrinterServerInterface::post_request(const QString &path, const QJsonObject &obj)
{
    QUrl url = path;
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonArray cipher_text;
    encrypt(QJsonDocument(obj).toJson(QJsonDocument::Compact), cipher_text);
    QNetworkReply *reply = g_networkManager.post(request,
            QJsonDocument(cipher_text).toJson(QJsonDocument::Compact));
    return reply;
}

void PrinterServerInterface::encrypt(const QString &text, QJsonArray &res)
{
    static const char pub_key[] = "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA1O6fa/bSzcQEQ/7B2wme\n"
        "6k+pNuuu3O4J4PpJyXmVMuQ8uGrnHv2GEiDMN4hGXRJK49yciIUyZvjtS1VDNiLy\n"
        "shiYGBebWeLcN8GcSuDjf2ZjWw3dVhi8lyjkDdxOWTnP6enCbRjZt5IiwjniYcK0\n"
        "b9eiAExkUCzMWvOU46f62RxaNj0HLzn8gIRARVGyu1wwqXLIdKlzsAew8G3GaFNC\n"
        "bLeDiQe3Uu9JfSOLf8IHG4gfHaNZpA0f3DfUXsOX70uDzSDzV80trFpPyl2DUhUK\n"
        "7s5zyKmm1iuooV79sYs8efZbP3p+Uy2Q2o2JEilOkXfxaSG1IDzvIfX+bnPHN2jO\n"
        "twIDAQAB";

    AutoSeededRandomPool rng;
    RSAES_OAEP_SHA_Encryptor pubkey;
    int maxlen;

    StringSource keybase64(pub_key, true, new Base64Decoder);
    pubkey = RSAES_OAEP_SHA_Encryptor(keybase64);

    QByteArray clear = text.toUtf8();
    const char* data = clear.constData();
    int len = clear.size();
    maxlen = pubkey.FixedMaxPlaintextLength();
    for (int i = len, j = 0; i > 0; i -= maxlen, j += maxlen) {
        std::string pieces;
        StringSource((unsigned char*)data + j, i > maxlen ? maxlen : i, true,
                new PK_EncryptorFilter(rng, pubkey,
                    new Base64Encoder(new StringSink(pieces))));
        res.append(pieces.c_str());
    }
}

PrinterService* PrinterService::getInstance()
{
    static PrinterService* instance = nullptr;
    if (nullptr == instance)
        instance = new PrinterService();

    return instance;
}

bool PrinterService::isInvaild()
{
    return m_hostname.isEmpty();
}

PrinterService::PrinterService()
{
    m_hostname = g_Settings->getHostName();
    if (m_hostname.isEmpty()) return;

    m_port = g_Settings->getHostPort();
    m_version = g_Settings->getClientVersion();
    m_code = g_Settings->getClientCode();
    m_osVersion = g_Settings->getOSVersion();

    if (m_osVersion.isEmpty())
        m_urlPrefix = QString("http://%1:%2").arg(m_hostname).arg(m_port);
    else
        m_urlPrefix = QString("http://%1:%2/%3").arg(m_hostname).arg(m_port).arg(m_osVersion);
}

PrinterServerInterface* PrinterService::searchSolution(const QString& manufacturer,
        const QString& model, const QString& ieee1284_id)
{
    QJsonObject obj = {
        QPair<QString, QJsonValue>(SD_KEY_ver, m_version),
        QPair<QString, QJsonValue>(SD_KEY_code, m_code),
        QPair<QString, QJsonValue>(SD_KEY_mfg, manufacturer),
        QPair<QString, QJsonValue>(SD_KEY_mdl, model),
        QPair<QString, QJsonValue>(SD_KEY_ieeeid, ieee1284_id),
    };

    if (isInvaild()) return nullptr;

    qDebug() << "search solution for " << manufacturer << " " << model << " " << ieee1284_id;
    PrinterServerInterface *reply = new PrinterServerInterface(m_urlPrefix+"/search", obj);

    return reply;
}

PrinterServerInterface* PrinterService::searchDriver(int solution_id)
{
    QJsonObject obj = {
        QPair<QString, QJsonValue>(SD_KEY_sid, solution_id),
    };

    if (isInvaild()) return nullptr;

    PrinterServerInterface *reply = new PrinterServerInterface(m_urlPrefix+"/driver", obj);

    return reply;
}

PrinterServerInterface* PrinterService::feedbackResult(int solution_id, bool success,
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

    if (isInvaild()) return nullptr;

    PrinterServerInterface *reply = new PrinterServerInterface(m_urlPrefix+"/report", obj);

    return reply;
}

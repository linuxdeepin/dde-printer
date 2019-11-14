#ifndef SERVICE_H
#define SERVICE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QJsonObject>

#define SD_KEY_from         "from"
#define SD_KEY_make_model   "describe"
#define SD_KEY_driver       "driver"
#define SD_KEY_excat        "excat"
#define SD_KEY_ppd          "ppd"
#define SD_KEY_sid          "sid"
#define SD_KEY_ver          "version"
#define SD_KEY_code         "client_code"
#define SD_KEY_mfg          "MFG"
#define SD_KEY_mdl          "MDL"
#define SD_KEY_ieeeid       "ieee1284_id"
#define SD_KEY_suc          "success"
#define SD_KEY_rid          "id"
#define SD_KEY_reason       "reason"
#define SD_KEY_feedback     "feedback"
#define SD_KEY_detail       "detail"
#define SD_KEY_package      "package"

class PrinterServerInterface : public QObject
{
    Q_OBJECT

public:
    PrinterServerInterface(const QString &url, const QJsonObject &obj, QObject* parent=nullptr);

    void postToServer();

signals:
    void signalDone(int, QByteArray);

private:
    QNetworkReply* post_request(const QString &path, const QJsonObject &obj);
    void encrypt(const QString &text, QJsonArray &res);

    QString                 m_url;
    QJsonObject             m_args;
};

class PrinterService : public QObject
{
    Q_OBJECT

public:
    static PrinterService* getInstance();

    PrinterServerInterface* searchSolution(const QString& manufacturer, const QString& model,
            const QString& ieee1284_id = "");

    PrinterServerInterface* searchDriver(int solution_id);

    PrinterServerInterface* feedbackResult(int solution_id, bool success,
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

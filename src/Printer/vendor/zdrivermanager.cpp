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

#include "zdrivermanager.h"
#include "cupsconnection.h"
#include "cupsattrnames.h"
#include "qtconvert.h"
#include "config.h"
#include "common.h"
#include "ztaskinterface.h"
#include "printerservice.h"
#include "zdrivermanager_p.h"
#include "cupsppd.h"

#ifdef CONSOLE_CMD
#include "zprintermanager.h"
#else
#include "dprintermanager.h"
#endif

#include <QProcess>
#include <QTcpSocket>
#include <QString>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QStringList>
#include <QMutexLocker>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QEventLoop>
#include <QTimer>

static QMutex g_mutex;
static QMap<QString, QMap<QString, QString>> g_ppds;//所以ppd文件的字典，以device_id(没有device_id则以make_and_model)作为key
static QMap<QString, QMap<QString, QString>> g_ppdsDirct;//将厂商和型号格式化之后作为key生成的字典，键值为g_ppds的key
static QMap<QString, QMap<QString, QString>> g_ppdsMakeModelNames;//厂商和型号的字典，用于显示厂商和型号列表
static QMap<QString, QString>                g_textPPd;//没有找到驱动的情况，默认的驱动

static int g_iStatus = TStat_None;

static QList<QMap<QString, QString>> getPPDNamesFromDeviceID(const QString &strUri, const QString &strMFG, const QString &strMDL, const QString &strCMD)
{
    QList<QMap<QString, QString>> list;
    QString strMake, strModel;
    QStringList strKeys;

    if (strMFG.isEmpty() || strMDL.isEmpty()) return list;

    qInfo() << QString("Find driver for %1, %2#%3#%4").arg(strUri)
        .arg(strMFG).arg(strMDL).arg(strCMD);

    strMake = normalize(strMFG);
    strModel = normalize(strMDL);
    strKeys = g_ppdsDirct.value(strMake).values(strModel);
    if (!strKeys.isEmpty())
    {
        foreach (QString strKey, strKeys) {
            QList<QMap<QString, QString>> flist = g_ppds.values(strKey.toLower());
            list += flist;
            qInfo() << QString("Match %1 drivers, got %2 for %3, %4")
                       .arg(flist.count()).arg(strKey).arg(strModel).arg(strMake);
        }
    }

    if (!strCMD.isEmpty())
    {
        QList<QMap<QString, QString>> templist;
        QStringList commandsets = strCMD.split(",");
        qDebug() << QString("Device commandsets: %1").arg(strCMD);
        //判断找到的驱动commandsets信息符不符合
        for(int i=0;i<list.count();i++) {
            QString devId = list[i].value(CUPS_PPD_ID);
            QMap<QString, QString> devDirct = parseDeviceID(devId);
            QString strcmd = devDirct.value("CMD");
            qDebug() << QString("PPD commandsets: %1").arg(strcmd);
            QStringList cmds = strcmd.split(",");
            foreach (QString cmd, cmds) {
                if(commandsets.contains(cmd))
                {
                    templist.append(list[i]);
                    break;
                }
            }
        }

        //如果所有的驱动都不符合，保留驱动，如果只是部分不符合，去掉不符合的驱动
        if(!templist.isEmpty())
        {
            int count = list.count();
            list = templist;
            if (count != list.count())
                qInfo() << QString("Remove some ppds, that commandsets not support %1").arg(strCMD);
        }
    }

    if (list.isEmpty())
        qWarning()<< QString("Not found driver %1, %2#%3#%4").arg(strUri).arg(strMFG).arg(strMDL).arg(strCMD);

    return list;
}

static QList<QMap<QString, QVariant>> stringToVariant(const QList<QMap<QString, QString>>& drivers)
{
    QList<QMap<QString, QVariant>> list;
    foreach (auto value, drivers) {
        QMap<QString, QVariant> info;
        QStringList keys = value.keys();
        foreach (QString key, keys) {
            info.insert(key, value.value(key));
        }
        list.append(info);
    }

    return list;
}

static bool addToDirct(const QString &strMake, const QString &strModel, const QString &key)
{
    QString makel, model;

    if (key.isEmpty()) {
        qWarning()<< QString("Dirct key is empty");
        return false;
    }

    makel = normalize(strMake);
    model = normalize(strModel);
    if (makel.isEmpty() || model.isEmpty()) {
        qWarning()<< QString("make_and_model failed for %1").arg(key);
        return false;
    }

    QMap<QString, QString> map = g_ppdsDirct.value(makel);
    QString strkey = map.value(model);
    if (strkey.isEmpty() || !g_driverManager->isSamePPD(key, strkey)) {
        map.insertMulti(model, key);
        g_ppdsDirct.insert(makel, map);
        qDebug() << QString("Insert %1#%2#%3 to dirct").arg(makel).arg(model).arg(key);
        return true;
    } else {
        qDebug() << "Remove duplicate driver" << strMake << strModel << strkey << "->" << key;
        return  false;
    }
}

static void getPpdMakeModel(QString &strMake, QString &strModel, QMap<QString, QString>& list)
{
    //先通过device id解析make model
    QString key = list.value(CUPS_PPD_ID);
    QMap<QString, QString> idInfo = parseDeviceID(key);
    strMake = idInfo["MFG"];
    strModel = idInfo["MDL"];
    ppdMakeModelSplit(strMake+" "+strModel, strMake, strModel);

    //如果无效，再通过ppd-make-and-model解析make model
    if (strMake.isEmpty() || strModel.isEmpty())
    {
        //qDebug() << "get make and model from strMakeAndModel";
        QString strMakeAndModel = list.value(CUPS_PPD_MAKE_MODEL);
        if (!strMakeAndModel.isEmpty())
        {
            QString make, model;
            ppdMakeModelSplit(strMakeAndModel, make, model);
            //qDebug() << QString("split MakeAndModel make and model: %1 , %2").arg(make).arg(model);
            if(strModel.isEmpty()) strModel = model;
            if(strMake.isEmpty()) strMake = make;
        }
    }

    //如果无效，再通过ppd-product解析make model
    if (strMake.isEmpty() || strModel.isEmpty())
    {
        //qDebug() << "get make and model from ppd-product";
        QString strProduct = list.value("ppd-product");
        QString prodMake, prodModel;
        if (!strProduct.isEmpty() && strProduct.startsWith("(") && strProduct.endsWith(")"))
        {
            QString lmake = normalize(strMake);
            strProduct = strProduct.mid(1, strProduct.length()-2);
            if (!strProduct.toLower().startsWith(lmake))
                strProduct = strMake +  " " + strProduct;

            ppdMakeModelSplit(strProduct, prodMake, prodModel);
            //qDebug() << QString("split Product make and model: %1 , %2").arg(prodMake).arg(prodModel);

            if(strModel.isEmpty()) strModel = prodModel;
            if(strMake.isEmpty()) strMake = prodMake;
        }
    }
}

int ReflushLocalPPDS::doWork()
{
    map<string, map<string, string>> allPPDS;
    map<string, map<string, string>>::iterator itall;

    qDebug() << QString("Starting...");

    QMutexLocker locker(&g_mutex);
    g_iStatus = TStat_Running;

    try {
        allPPDS = g_cupsConnection->getPPDs2(0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, -1, nullptr, nullptr, nullptr);
    }catch(const std::exception &ex) {
        qWarning() << "Got execpt: " << QString::fromUtf8(ex.what());
        return -1;
    };

    if (m_bQuit) return 0;

    g_ppds.clear();
    g_ppdsDirct.clear();
    g_ppdsMakeModelNames.clear();
    g_textPPd.clear();
    int count = 0;
    for (itall=allPPDS.begin();itall!=allPPDS.end();itall++) {
        qDebug() << QString("*****************************");
        QMap<QString, QString> list;
        map<string, string> mapValue = itall->second;
        map<string, string>::iterator itinfo = mapValue.begin();

        if (m_bQuit) return 0;

        QString ppdname = STQ(itall->first);
        qDebug() << CUPS_PPD_NAME << ":" << ppdname;
        list.insert(CUPS_PPD_NAME, ppdname);
        for (;itinfo!=mapValue.end();itinfo++) {
            QString attrName = STQ(itinfo->first);
            QString attrValue = attrValueToQString(itinfo->second);
            qDebug() << attrName << ":" << attrValue;
            list.insert(attrName, attrValue);
        }

        if (!ppdname.isEmpty()) {
            QString key = ppdname.toLower();
            QString strMake, strModel;
            getPpdMakeModel(strMake, strModel, list);

            count++;
            //make sure has make and model name
            if (strModel.isEmpty() || strMake.isEmpty() || key.isEmpty()){
                qWarning()<< QString("invaild ppd %1").arg(ppdname);
                continue;
            } else if (addToDirct(strMake, strModel, key)) {
                strMake = toNormalName(strMake);
                list.insert(CUPS_PPD_MODEL, strModel);
                list.insert(CUPS_PPD_MAKE, strMake);

                QMap<QString, QString> map = g_ppdsMakeModelNames.value(strMake);
                map.insertMulti(strModel, key);
                g_ppdsMakeModelNames.insert(strMake, map);

                if (g_textPPd.isEmpty() && (ppdname.endsWith("textonly.ppd") || ppdname.endsWith("postscript.ppd")))
                    g_textPPd = list;
            }

            g_ppds.insertMulti(key, list);
        }
        qDebug() << QString("*****************************");
    }

    qInfo() << QString("Got ppds count: %1").arg(count);
    g_iStatus = TStat_Suc;
    return 0;
}

DriverSearcher::DriverSearcher(const TDeviceInfo &printer, QObject *parent)
    :QObject(parent)
{
    m_printer = printer;
}

void DriverSearcher::startSearch()
{
    QString strMake, strModel;

    QMap<QString, QVariant> driver = g_driverManager->getEveryWhereDriver(m_printer.uriList[0]);
    if (!driver.isEmpty()) {
        m_drivers.append(driver);
    }

    getLocalDrivers();

    if(m_printer.strDeviceId.isEmpty())
        ppdMakeModelSplit(m_printer.strMakeAndModel, strMake, strModel);
    else
    {
        QMap<QString, QString> dirct = parseDeviceID(m_printer.strDeviceId);
        strMake = dirct.value("MFG");
        strModel = dirct.value("MDL");
    }

    PrinterServerInterface *search = g_printerServer->searchSolution(strMake, strModel, m_printer.strDeviceId);
    if (search) {
        connect(search, &PrinterServerInterface::signalDone, this, &DriverSearcher::slotDone);
        search->postToServer();
    } else {
        askForFinish();
    }
}

QList<QMap<QString, QVariant>> DriverSearcher::getDrivers()
{
    return m_drivers;
}

TDeviceInfo DriverSearcher::getPrinter()
{
    return m_printer;
}

static void insertDriver(QList<QMap<QString, QVariant>> &drivers, QStringList &ppdNames, const QMap<QString, QVariant> &driver, int index)
{
    bool isDup = false;
    QString ppdname = driver[CUPS_PPD_NAME].toString();
    foreach (QString str, ppdNames) {
        if (g_driverManager->isSamePPD(str, ppdname)) {
            qInfo() << "Remove same ppd" << ppdname;
            isDup = true;
        }
    }

    if (isDup) return;

    //将精确查找的结果排列到EveryWhere驱动之后,其他驱动之前
    if (driver[SD_KEY_from].toInt() == PPDFrom_Server && driver[SD_KEY_excat].toBool()) {
        drivers.insert(index, driver);
    } else {
        drivers.append(driver);
    }

    ppdNames.append(ppdname);
}

void DriverSearcher::sortDrivers()
{
    QList<QMap<QString, QVariant>> drivers;
    QStringList ppdNames;
    int index = 0;

    if (m_drivers.isEmpty()) return;

    //Everywhere 驱动放在第一位
    if (m_drivers[0][SD_KEY_from].toInt() == PPDFrom_EveryWhere) {
        drivers.append(m_drivers[0]);
        index++;
    }

    //先从本地驱动开始遍历，保证本地驱动在服务器返回的非精确查找之前
    for(int i=m_localIndex;i<m_drivers.count();i++) {
        insertDriver(drivers, ppdNames, m_drivers[i], index);
    }
    //查找驱动的时候如果本地驱动没有初始化完成，服务器返回的驱动可能在本地驱动前面
    for (int i=index;i<m_localIndex;i++) {
        insertDriver(drivers, ppdNames, m_drivers[i], index);
    }

    m_drivers = drivers;
}

void DriverSearcher::askForFinish()
{
    //如果之前本地驱动还没有初始化完成，则等待驱动初始化完成再搜索一次
    if (-1 == m_localIndex) {
        if (g_iStatus < TStat_Suc) {
            connect(g_driverManager, &DriverManager::signalStatus, this, &DriverSearcher::slotDriverInit);
            return;
        }

        getLocalDrivers();
    }

    sortDrivers();
    emit signalDone();
}

void DriverSearcher::slotDone(int iCode, const QByteArray &result)
{
    qDebug() << iCode << result;

    if (QNetworkReply::NoError == iCode && !result.isNull()) {
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(result, &err);
        qDebug() << doc.toJson();
        QJsonArray array =  doc.object()["solutions"].toArray();
        if (!array.isEmpty()) {
            foreach (QJsonValue value, array) {
                QJsonObject ppdobject = value.toObject();
                QMap<QString, QVariant> ppd;

                ppd.insert(SD_KEY_from, PPDFrom_Server);
                ppd.insert(CUPS_PPD_MAKE_MODEL, ppdobject.value(SD_KEY_make_model));
                ppd.insert(SD_KEY_driver, ppdobject.value(SD_KEY_driver));
                ppd.insert(SD_KEY_excat, ppdobject.value(SD_KEY_excat));
                ppd.insert(CUPS_PPD_NAME, ppdobject.value(SD_KEY_ppd));
                ppd.insert(SD_KEY_sid, ppdobject.value(SD_KEY_sid));

                m_drivers.append(ppd);
            }
        }
    }

    qInfo() << "Got driver count:" << m_drivers.count();
    askForFinish();
}

void DriverSearcher::slotDriverInit(int id, int state)
{
    Q_UNUSED(id);

    if (state >= TStat_Suc) {
        getLocalDrivers();
        sortDrivers();
        emit signalDone();
    }
}

int DriverSearcher::getLocalDrivers()
{
    QMap<QString, QString> id_dirct;
    QString strMake, strModel;

    if (g_iStatus < TStat_Suc) {
        qWarning() << "PPD not init";
        m_localIndex = -1;
        return -1;
    }

    m_localIndex = m_drivers.count();
    if (m_printer.strMakeAndModel.isEmpty() && m_printer.strDeviceId.isEmpty()) {
        qWarning() << "printer info is invaild";
        return -2;
    }

    QMutexLocker locker(&g_mutex);
    qInfo() << QString("Find driver for %1, %2, %3").arg(m_printer.uriList[0]).arg(m_printer.strMakeAndModel).arg(m_printer.strDeviceId);

    if (g_ppdsDirct.isEmpty() || g_ppds.isEmpty())
    {
        qWarning() << QString("PPD dirct is empty");
        return -3;
    }

    //通过device id中的MFG、MDL、CMD字段匹配
    if (!m_printer.strDeviceId.isEmpty())
    {
        QList<QMap<QString, QString>> list1;
        id_dirct = parseDeviceID(m_printer.strDeviceId);
        strMake = id_dirct["MFG"];
        strModel = id_dirct["MDL"];
        ppdMakeModelSplit(strMake+" "+strModel, strMake, strModel);
        list1 = getPPDNamesFromDeviceID(m_printer.uriList[0], strMake, strModel, id_dirct["CMD"]);
        m_drivers += stringToVariant(list1);
    }

    //如果没有找到再尝试通过make_and_model解析出来的make和model匹配
    if (!m_printer.strMakeAndModel.isEmpty() && m_drivers.isEmpty())
    {
        QList<QMap<QString, QString>> list1;
        ppdMakeModelSplit(m_printer.strMakeAndModel, strMake, strModel);
        list1 = getPPDNamesFromDeviceID(m_printer.uriList[0], strMake, strModel, "");
        m_drivers += stringToVariant(list1);
    }

    qInfo() << QString("Got %1 drivers").arg(m_drivers.count());

    return 0;
}

DriverManager* DriverManager::getInstance()
{
    static DriverManager* ppdInstance = nullptr;
    if (ppdInstance == nullptr)
    {
        ppdInstance = new DriverManager;
    }
    return ppdInstance;
}

DriverManager::DriverManager(QObject *parent) : QObject(parent)
{
    m_reflushTask = nullptr;
}

int DriverManager::getStatus()
{
    return g_iStatus;
}

int DriverManager::stop()
{
    if (m_reflushTask) m_reflushTask->stop();

    return 0;
}

DriverSearcher* DriverManager::createSearcher(const TDeviceInfo &device)
{
    DriverSearcher* searcher = new DriverSearcher(device, this);
    return searcher;
}

int DriverManager::reflushPpds()
{
    if (TStat_Running == g_iStatus || m_reflushTask)
        return 0;

    m_reflushTask = new ReflushLocalPPDS(this);
    connect(m_reflushTask, &ReflushLocalPPDS::signalStatus, this, &DriverManager::signalStatus);
    m_reflushTask->start();

    return 0;
}

QMap<QString, QMap<QString, QString>>* DriverManager::getMakeModelNames()
{
    if(TStat_Suc > g_iStatus){
        qWarning() << "PPD is not inited";
        return nullptr;
    }

    return &g_ppdsMakeModelNames;
}

QMap<QString, QMap<QString, QString>>* DriverManager::getPPDs()
{
    if(TStat_Suc > g_iStatus){
        qWarning() << "PPD is not inited";
        return nullptr;
    }

    return &g_ppds;
}

QMap<QString, QString> DriverManager::getTextPPD()
{
    if(TStat_Suc > g_iStatus)
        return QMap<QString, QString>();

    return g_textPPd;
}

QMap<QString, QVariant> DriverManager::getEveryWhereDriver(const QString &strUri)
{
    QMap<QString, QVariant> driver;

    /* LanguageEncoding in generated ppd file is always ISOLatin1,
     * but it may contains utf-8 encoded character, so we don't use
     * everywhere model unless this issuse solved.
     * CUPS issue #5362 */
    if ((strUri.startsWith("dnssd://") && strUri.contains("/cups")) ||
        ((strUri.startsWith("ipp://")||strUri.startsWith("ipps")) && strUri.contains("/printers"))) {
        driver.insert(SD_KEY_from, PPDFrom_EveryWhere);
        driver.insert(CUPS_PPD_MAKE_MODEL, tr("EveryWhere driver"));
        driver.insert(CUPS_PPD_NAME, "EveryWhere driver");
        qDebug() << "Got everywhere driver for" << strUri;
    }

    return driver;
}

QStringList DriverManager::getDriverDepends(const char* strPPD)
{
    PPD p;
    std::vector<Attribute> attrs;
    QStringList depends;

    try {
        if (QFile::exists(strPPD)) {
            p.load(strPPD);
        } else {
            string ppdfile = g_cupsConnection->getServerPPD(strPPD);

            qInfo() << strPPD << STQ(ppdfile);
            p.load(ppdfile.c_str());
        }
        attrs = p.getAttributes();
    }catch(const std::exception &ex) {
        qWarning() << "Got execpt: " << QString::fromUtf8(ex.what());
        return depends;
    };

    for (size_t i=0;i<attrs.size();i++) {
        QString strName = STQ(attrs[i].getName());
        QString strValue = STQ(attrs[i].getValue());
        qDebug() << strName << strValue;
        if (strName == "NickName") {
            QString strMake, strModel;
            ppdMakeModelSplit(strValue, strMake, strModel);
            if (strMake.toLower() == "hp" && strValue.contains("requires proprietary plugin")) {
                depends << "hplip-plugin";
            }
            break;
        }
    }

    qInfo() << strPPD << depends;
    return depends;
}

bool DriverManager::isSamePPD(const QString &ppd1, const QString &ppd2)
{
    if (ppd1.isEmpty() || ppd2.isEmpty()) return false;
    QString ppd1l = ppd1.toLower();
    QString ppd2l = ppd2.toLower();
    if(ppd1l.endsWith(ppd2l) || ppd2l.endsWith(ppd1l))
        return true;

    return false;
}



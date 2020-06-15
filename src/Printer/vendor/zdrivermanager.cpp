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
#include "dprintermanager.h"

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
static QMap<QString, QMap<QString, QString>> g_ppds; //所以ppd文件的字典，以device_id(没有device_id则以make_and_model)作为key
static QMap<QString, QMap<QString, QString> *> g_ppdsDirct; //将厂商和型号格式化之后作为key生成的字典，键值为g_ppds的key
static QMap<QString, QMap<QString, QString> *> g_ppdsMakeModelNames; //厂商和型号的字典，用于显示厂商和型号列表
static QMap<QString, QString> g_textPPd; //没有找到驱动的情况，默认的驱动

static int g_iStatus = TStat_None;

static QMap<QString, QVariant> stringToVariant(const QMap<QString, QString> &driver)
{
    QMap<QString, QVariant> info;
    QStringList keys = driver.keys();
    foreach (QString key, keys) {
        info.insert(key, driver.value(key));
    }
    return info;
}

static QStringList findBestMatchPPDs(QStringList &models, QString mdl)
{
    QStringList list;
    int index = 0, len = 0;
    QString left, right;
    int leftMatch = 0, rightMatch = 0;
    int leftlen = 0, rightlen = 0;

    //先通过字符串排序找到和mdl最相近的项
    mdl = mdl.toLower();
    models.append(mdl);
    models.sort();
    index = models.indexOf(mdl);
    len = models.count();
    if (index > 0)
        left = models[index - 1];
    if (index < (len - 1))
        right = models[index + 1];

    qDebug() << QString("Between %1 left: %2 righ: %3").arg(mdl).arg(left).arg(right);

    //找出左右两边和mdl匹配的字符数目
    len = mdl.length();
    leftlen = left.length();
    rightlen = right.length();
    foreach (QChar ch, mdl) {
        if (leftMatch > -1 && leftMatch < leftlen && left[leftMatch] == ch)
            leftMatch++;
        else
            leftlen = -1;

        if (rightMatch > -1 && rightMatch < rightlen && right[rightMatch] == ch)
            rightMatch++;
        else
            rightlen = -1;

        if (rightlen < 1 && leftlen < 1)
            break;
    }

    //取匹配字符数较多并且超过mdl一半的项
    if (rightMatch > leftMatch && rightMatch > len / 2)
        list.append(right);
    else if (leftMatch > rightMatch && leftMatch > len / 2)
        list.append(left);
    else {
        //如果没有匹配到，取字符串里面的数字进行匹配
        models.removeAll(mdl);
        QStringList letters = mdl.split(" ");
        QString number;
        foreach (QString str, letters) {
            if (str[0].isDigit()) {
                number = str;
                break;
            }
        }
        if (!number.isEmpty()) {
            foreach (QString str, models) {
                letters = str.split(" ");
                if (letters.contains(number))
                    list.append(str);
            }
        }
    }

    qDebug() << QString("Got %1").arg(list.join(","));
    return list;
}

static QStringList getPPDNameFromCommandSet(QString strCMD)
{
    QStringList list, cmds;
    QStringList models;
    QStringList mdls;
    QMap<QString, QString> *pmodels = g_ppdsDirct.value("generic");

    if (pmodels)
        models = pmodels->keys();

    strCMD = strCMD.toLower();
    cmds = strCMD.split(",");
    if (cmds.contains("postscript") || cmds.contains("postscript2")
        || cmds.contains("postscript level 2 emulation"))
        mdls << "postscript";
    else if (cmds.contains("pclxl") || cmds.contains("pcl-xl") || cmds.contains("pcl6") || cmds.contains("pcl 6 emulation"))
        mdls << "PCL 6/PCL XL"
             << "PCL Laser";
    else if (cmds.contains("pcl5e"))
        mdls << "PCL 5e"
             << "PCL Laser";
    else if (cmds.contains("pcl5c"))
        mdls << "PCL 5c"
             << "PCL Laser";
    else if (cmds.contains("pcl5"))
        mdls << "PCL 5"
             << "PCL Laser";
    else if (cmds.contains("pcl"))
        mdls << "PCL 3"
             << "PCL Laser";
    if (cmds.contains("escpl2") || cmds.contains("esc/p2")
        || cmds.contains("escp2e"))
        mdls << "ESC/P Dot Matrix";

    foreach (QString mdl, mdls) {
        qDebug() << QString("Find for CMD: %1, mdl: %2").arg(strCMD).arg(mdl);
        list += findBestMatchPPDs(models, mdl);
    }

    return list;
}

static QList<QMap<QString, QVariant>> getFuzzyMatchDrivers(const QString &strMake, const QString &strModel, const QString &strCMD)
{
    QList<QMap<QString, QVariant>> list;

    if (strMake.isEmpty() || strModel.isEmpty()) {
        qWarning() << "printer info is invaild";
        return list;
    }

    if (g_ppdsDirct.contains(strMake)) {
        QMap<QString, QString> *modelMap = g_ppdsDirct.value(strMake);
        if (!modelMap)
            return list;

        QStringList modellist = modelMap->keys();
        QStringList models = findBestMatchPPDs(modellist, strModel);
        foreach (QString mdl, models) {
            QString key = modelMap->value(mdl);
            QMap<QString, QVariant> driver = stringToVariant(g_ppds.value(key.toLower()));
            if (!driver.isEmpty()) {
                driver.insert(SD_KEY_excat, false);
                list.append(driver);
            }
        }
    }

    if (!strCMD.isEmpty()) {
        QList<QMap<QString, QVariant>> templist;
        QStringList commandsets = strCMD.split(",");
        qDebug() << QString("Device commandsets: %1").arg(strCMD);
        //判断找到的驱动commandsets信息符不符合
        for (int i = 0; i < list.count(); i++) {
            QString devId = list[i].value(CUPS_PPD_ID).toString();
            QMap<QString, QString> devDirct = parseDeviceID(devId);
            QString strcmd = devDirct.value("CMD");
            qDebug() << QString("PPD commandsets: %1").arg(strcmd);
            QStringList cmds = strcmd.split(",");
            foreach (QString cmd, cmds) {
                if (commandsets.contains(cmd)) {
                    templist.append(list[i]);
                    break;
                }
            }
        }

        //如果所有的驱动都不符合，保留驱动，如果只是部分不符合，去掉不符合的驱动
        if (!templist.isEmpty()) {
            int count = list.count();
            list = templist;
            if (count != list.count())
                qDebug() << QString("Remove some ppds, that commandsets not support %1").arg(strCMD);
        }

        if (list.isEmpty()) {
            //通过commandsets查找驱动
            QStringList models = getPPDNameFromCommandSet(strCMD);
            foreach (QString mdl, models) {
                QMap<QString, QString> *modelMap = g_ppdsDirct.value(strMake);
                if (modelMap) {
                    QString key = modelMap->value(mdl);
                    QMap<QString, QVariant> driver = stringToVariant(g_ppds.value(key.toLower()));
                    if (!driver.isEmpty()) {
                        driver.insert(SD_KEY_excat, false);
                        list.append(driver);
                    }
                }
            }
        }
    }

    qInfo() << "Got dirver count: " << list.count();
    return list;
}

static QList<QMap<QString, QVariant>> getExactMatchDrivers(const QString &strMFG, const QString &strMDL)
{
    QList<QMap<QString, QVariant>> list;
    QString strMake, strModel;
    QStringList strKeys;

    if (strMFG.isEmpty() || strMDL.isEmpty())
        return list;

    QMap<QString, QString> *modelMap = g_ppdsDirct.value(strMake);
    if (!modelMap)
        return list;

    strKeys = modelMap->values(strModel);
    if (!strKeys.isEmpty()) {
        foreach (QString strKey, strKeys) {
            QMap<QString, QVariant> driver = stringToVariant(g_ppds.value(strKey.toLower()));
            if (!driver.isEmpty()) {
                driver.insert(SD_KEY_excat, true);
                list.append(driver);
            }
        }
    }

    qInfo() << "Got driver count: " << list.count();

    return list;
}

static bool addToDirct(const QString &strMake, const QString &strModel, const QString &key)
{
    QString makel, model;

    if (key.isEmpty()) {
        qWarning() << QString("Dirct key is empty");
        return false;
    }

    makel = normalize(strMake);
    model = normalize(strModel);
    if (makel.isEmpty() || model.isEmpty()) {
        qWarning() << QString("make_and_model failed for %1").arg(key);
        return false;
    }

    QMap<QString, QString> *modelMap = g_ppdsDirct.value(makel);
    if (!modelMap) {
        modelMap = new QMap<QString, QString>();
        g_ppdsDirct.insert(makel, modelMap);
    }

    QString strkey = modelMap->value(model);
    if (strkey.isEmpty() || !g_driverManager->isSamePPD(key, strkey)) {
        modelMap->insertMulti(model, key);
        qDebug() << QString("Insert %1#%2#%3 to dirct").arg(makel).arg(model).arg(key);
        return true;
    } else {
        qDebug() << "Remove duplicate driver" << strMake << strModel << strkey << "->" << key;
        return false;
    }
}

static void getPpdMakeModel(QString &strMake, QString &strModel, QMap<QString, QString> &list)
{
    //先通过device id解析make model
    QString key = list.value(CUPS_PPD_ID);
    QMap<QString, QString> idInfo = parseDeviceID(key);
    strMake = idInfo["MFG"];
    strModel = idInfo["MDL"];
    ppdMakeModelSplit(strMake + " " + strModel, strMake, strModel);

    //如果无效，再通过ppd-make-and-model解析make model
    if (strMake.isEmpty() || strModel.isEmpty()) {
        //qDebug() << "get make and model from strMakeAndModel";
        QString strMakeAndModel = list.value(CUPS_PPD_MAKE_MODEL);
        if (!strMakeAndModel.isEmpty()) {
            QString make, model;
            ppdMakeModelSplit(strMakeAndModel, make, model);
            //qDebug() << QString("split MakeAndModel make and model: %1 , %2").arg(make).arg(model);
            if (strModel.isEmpty())
                strModel = model;
            if (strMake.isEmpty())
                strMake = make;
        }
    }

    //如果无效，再通过ppd-product解析make model
    if (strMake.isEmpty() || strModel.isEmpty()) {
        //qDebug() << "get make and model from ppd-product";
        QString strProduct = list.value("ppd-product");
        QString prodMake, prodModel;
        if (!strProduct.isEmpty() && strProduct.startsWith("(") && strProduct.endsWith(")")) {
            QString lmake = normalize(strMake);
            strProduct = strProduct.mid(1, strProduct.length() - 2);
            if (!strProduct.toLower().startsWith(lmake))
                strProduct = strMake + " " + strProduct;

            ppdMakeModelSplit(strProduct, prodMake, prodModel);
            //qDebug() << QString("split Product make and model: %1 , %2").arg(prodMake).arg(prodModel);

            if (strModel.isEmpty())
                strModel = prodModel;
            if (strMake.isEmpty())
                strMake = prodMake;
        }
    }
}

int RefreshLocalPPDS::doWork()
{
    map<string, map<string, string>> allPPDS;
    map<string, map<string, string>>::iterator itall;

    qDebug() << QString("Starting...");

    QMutexLocker locker(&g_mutex);
    g_iStatus = TStat_Running;

    try {
        allPPDS = g_cupsConnection->getPPDs2(0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, -1, nullptr, nullptr, nullptr);
    } catch (const std::exception &ex) {
        qWarning() << "Got execpt: " << QString::fromUtf8(ex.what());
        return -1;
    };

    if (m_bQuit)
        return 0;

    int count = 0;
    qInfo() << "format ppd info";
    for (itall = allPPDS.begin(); itall != allPPDS.end(); itall++) {
        qDebug() << QString("*****************************");
        QMap<QString, QString> list;
        map<string, string> mapValue = itall->second;
        map<string, string>::iterator itinfo = mapValue.begin();

        if (m_bQuit)
            return 0;

        QString ppdname = STQ(itall->first);
        qDebug() << CUPS_PPD_NAME << ":" << ppdname;
        list.insert(CUPS_PPD_NAME, ppdname);
        for (; itinfo != mapValue.end(); itinfo++) {
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
            if (strModel.isEmpty() || strMake.isEmpty() || key.isEmpty()) {
                qWarning() << QString("invaild ppd %1").arg(ppdname);
                continue;
            } else if (addToDirct(strMake, strModel, key)) {
                strMake = toNormalName(strMake);
                strModel = toNormalName(strModel);
                list.insert(CUPS_PPD_MODEL, strModel);
                list.insert(CUPS_PPD_MAKE, strMake);

                QMap<QString, QString> *modelMap = g_ppdsMakeModelNames.value(strMake);
                if (!modelMap) {
                    modelMap = new QMap<QString, QString>();
                    g_ppdsMakeModelNames.insert(strMake, modelMap);
                }
                modelMap->insertMulti(strModel, key);
                ;

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
    : QObject(parent)
{
    m_printer = printer;
    m_localIndex = -1;

    //通过device id中的MFG、MDL、CMD字段匹配
    if (!m_printer.strDeviceId.isEmpty()) {
        QMap<QString, QString> id_dirct;
        id_dirct = parseDeviceID(m_printer.strDeviceId);
        m_strCMD = id_dirct["CMD"];
        m_strMake = id_dirct["MFG"];
        m_strModel = id_dirct["MDL"];
        ppdMakeModelSplit(m_strMake + " " + m_strModel, m_strMake, m_strModel);
    } else if (!m_printer.strMakeAndModel.isEmpty()) {
        //如果没有device id,尝试通过make_and_model解析出来的make和model匹配
        ppdMakeModelSplit(m_printer.strMakeAndModel, m_strMake, m_strModel);
    }

    qInfo() << QString("Find driver for %1, %2, %3").arg(m_printer.uriList[0]).arg(m_printer.strMakeAndModel).arg(m_printer.strDeviceId);
}

void DriverSearcher::startSearch()
{
    QString strMake, strModel;

    QMap<QString, QVariant> driver = g_driverManager->getEveryWhereDriver(m_printer.uriList[0]);
    if (!driver.isEmpty()) {
        m_drivers.append(driver);
        qInfo() << "Got EveryWhere driver";
        emit signalDone();
    }

    /*等待服务器查找结果返回之后再开始查找本地驱动
     * 如果服务有精确查找到驱动，则不需要从本地查找
    */

    PrinterServerInterface *search = g_printerServer->searchSolution(m_strMake, m_strModel, m_printer.strDeviceId);
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

static void insertDriver(QList<QMap<QString, QVariant>> &drivers, QStringList &ppdNames, const QMap<QString, QVariant> &driver, int headerIndex)
{
    bool isDup = false;
    QString ppdname = driver[CUPS_PPD_NAME].toString();
    foreach (QString str, ppdNames) {
        if (g_driverManager->isSamePPD(str, ppdname)) {
            qInfo() << "Remove same ppd" << ppdname;
            isDup = true;
        }
    }

    if (isDup)
        return;

    //将精确查找的结果排列到EveryWhere驱动之后,其他驱动之前
    if (driver[SD_KEY_from].toInt() == PPDFrom_Server && driver[SD_KEY_excat].toBool()) {
        drivers.insert(headerIndex, driver);
    } else {
        drivers.append(driver);
    }

    ppdNames.append(ppdname);
}

void DriverSearcher::sortDrivers()
{
    QList<QMap<QString, QVariant>> drivers;
    QStringList ppdNames;
    int headerIndex = 0;

    if (m_drivers.isEmpty())
        return;

    //没有通过本地查找的驱动，不需要进行排序
    if (m_localIndex < 0 || m_localIndex >= m_drivers.count()) {
        return;
    }

    //Everywhere 驱动放在第一位
    if (m_drivers[0][SD_KEY_from].toInt() == PPDFrom_EveryWhere) {
        drivers.append(m_drivers[0]);
        headerIndex++;
    }

    /*先从本地查找的驱动开始遍历，保证本地驱动先插入列表中
     * 然后遍历从服务获取的驱动，如果服务器获取的是精确查找的结果，则可以插入到本地驱动之前
     * 如果服务器获取的不是精确查找的结果，则插入到本地驱动之后
     * 如果和本地驱动重复，则删除服务器查找的结果
    */
    for (int i = m_localIndex; i < m_drivers.count(); i++) {
        insertDriver(drivers, ppdNames, m_drivers[i], headerIndex);
    }
    for (int i = headerIndex; i < m_localIndex; i++) {
        insertDriver(drivers, ppdNames, m_drivers[i], headerIndex);
    }

    m_drivers = drivers;
}

bool DriverSearcher::hasExcatDriver()
{
    foreach (auto driver, m_drivers) {
        if (driver[SD_KEY_excat].toBool()) {
            return true;
        }
    }

    return false;
}

void DriverSearcher::askForFinish()
{
    /*首次尝试通过本地和服务器查找驱动之后，可能本地驱动还没有初始化完成。
     * 如果首次没有查找到精确匹配的驱动，等待本地驱动初始化完成之后再执行一次本地精确查找
     * 如果本地精确查找仍然没有找到驱动，在执行一次模糊查找
    */
    if (!hasExcatDriver()) {
        if (-1 == m_localIndex) {
            if (g_iStatus < TStat_Suc) {
                qInfo() << "Wait ppd init";
                connect(g_driverManager, &DriverManager::signalStatus, this, &DriverSearcher::slotDriverInit, Qt::UniqueConnection);
                return;
            }

            //驱动初始化完成，再执行一次本地精确查找
            getLocalDrivers();
        }

        if (TStat_Suc == g_iStatus && !hasExcatDriver() && (!m_strMake.isEmpty() || !m_strModel.isEmpty())) {
            QMutexLocker locker(&g_mutex);

            m_strMake = normalize(m_strMake);
            m_strModel = normalize(m_strModel);
            QList<QMap<QString, QVariant>> list = getFuzzyMatchDrivers(m_strMake, m_strModel, m_strCMD);
            if (!list.isEmpty()) {
                m_localIndex = m_drivers.count();
                m_drivers += list;
            }
        }
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
        QJsonArray array = doc.object()["solutions"].toArray();
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
        askForFinish();
    }
}

int DriverSearcher::getLocalDrivers()
{
    QString strMake, strModel;

    if (g_iStatus < TStat_Suc) {
        qWarning() << "PPD not init";
        m_localIndex = -1;
        return -1;
    }

    if (m_strMake.isEmpty() || m_strModel.isEmpty()) {
        qWarning() << "printer info is invaild";
        return -2;
    }

    strMake = normalize(m_strMake);
    strModel = normalize(m_strModel);
    QMutexLocker locker(&g_mutex);

    if (g_ppdsDirct.isEmpty() || g_ppds.isEmpty()) {
        qWarning() << QString("PPD dirct is empty");
        return -3;
    }

    QList<QMap<QString, QVariant>> list = getExactMatchDrivers(strMake, strModel);
    if (!list.isEmpty()) {
        m_localIndex = m_drivers.count();
        m_drivers += list;
    }

    qInfo() << QString("Got %1 drivers").arg(list.count());

    return list.count();
}

DriverManager *DriverManager::getInstance()
{
    static DriverManager *ppdInstance = nullptr;
    if (ppdInstance == nullptr) {
        ppdInstance = new DriverManager;
    }
    return ppdInstance;
}

DriverManager::DriverManager(QObject *parent)
    : QObject(parent)
{
    m_refreshTask = nullptr;
}

int DriverManager::getStatus()
{
    return g_iStatus;
}

int DriverManager::stop()
{
    if (m_refreshTask) {
        m_refreshTask->stop();
        m_refreshTask->deleteLater();
        m_refreshTask = nullptr;
    }

    QMutexLocker locker(&g_mutex);
    g_iStatus = TStat_None;
    qInfo() << "Clear local driver dirct";

    QList<QMap<QString, QString> *> models = g_ppdsDirct.values();
    foreach (auto model, models) {
        if (model)
            delete model;
    }
    models = g_ppdsMakeModelNames.values();
    foreach (auto model, models) {
        if (model)
            delete model;
    }
    g_ppds.clear();
    g_ppdsDirct.clear();
    g_ppdsMakeModelNames.clear();
    g_textPPd.clear();
    return 0;
}

DriverSearcher *DriverManager::createSearcher(const TDeviceInfo &device)
{
    DriverSearcher *searcher = new DriverSearcher(device, this);
    return searcher;
}

int DriverManager::refreshPpds()
{
    QMutexLocker locker(&g_mutex);
    if (TStat_Running == g_iStatus || m_refreshTask)
        return 0;

    qInfo() << "";
    m_refreshTask = new RefreshLocalPPDS();
    connect(m_refreshTask, &RefreshLocalPPDS::signalStatus, this, &DriverManager::signalStatus);
    m_refreshTask->start();

    return 0;
}

QStringList DriverManager::getAllMakes()
{
    QStringList makes;

    if (TStat_Suc == g_iStatus) {
        makes = g_ppdsMakeModelNames.keys();
    }

    return makes;
}

const QMap<QString, QString> *DriverManager::getModelsByMake(const QString &strMake)
{
    if (TStat_Suc > g_iStatus) {
        qWarning() << "PPD is not inited";
        return nullptr;
    }

    return g_ppdsMakeModelNames.value(strMake);
}

const QMap<QString, QMap<QString, QString>> *DriverManager::getPPDs()
{
    if (TStat_Suc > g_iStatus) {
        qWarning() << "PPD is not inited";
        return nullptr;
    }

    return &g_ppds;
}

QMap<QString, QString> DriverManager::getTextPPD()
{
    if (TStat_Suc > g_iStatus)
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
    if ((strUri.startsWith("dnssd://") && strUri.contains("/cups")) || ((strUri.startsWith("ipp://") || strUri.startsWith("ipps")) && strUri.contains("/printers"))) {
        driver.insert(SD_KEY_from, PPDFrom_EveryWhere);
        driver.insert(CUPS_PPD_MAKE_MODEL, tr("EveryWhere driver"));
        driver.insert(CUPS_PPD_NAME, "EveryWhere driver");
        driver.insert(SD_KEY_excat, true);
        qDebug() << "Got everywhere driver for" << strUri;
    }

    return driver;
}

QStringList DriverManager::getDriverDepends(const char *strPPD)
{
    PPD p;
    std::vector<Attribute> attrs;
    QStringList depends;

    try {
        if (QFile::exists(strPPD)) {
            p.load(strPPD);
        } else {
            string ppdfile = g_cupsConnection->getServerPPD(strPPD);

            qDebug() << strPPD << STQ(ppdfile);
            p.load(ppdfile.c_str());
        }
        attrs = p.getAttributes();
    } catch (const std::exception &ex) {
        qWarning() << "Got execpt: " << QString::fromUtf8(ex.what());
        return depends;
    };

    for (size_t i = 0; i < attrs.size(); i++) {
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
    if (ppd1.isEmpty() || ppd2.isEmpty())
        return false;
    QString ppd1l = ppd1.toLower();
    QString ppd2l = ppd2.toLower();
    if (ppd1l.endsWith(ppd2l) || ppd2l.endsWith(ppd1l))
        return true;

    return false;
}

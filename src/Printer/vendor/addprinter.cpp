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

#include "addprinter.h"
#include "addprinter_p.h"
#include "zdrivermanager.h"
#include "cupsattrnames.h"
#include "ztaskinterface.h"
#include "printerservice.h"
#include "common.h"
#include "cupsconnection.h"
#include "qtconvert.h"
#include "dprintermanager.h"

#include <QRegularExpression>
#include <QFile>
#include <QProcess>
#include <QJsonParseError>
#include <QFileInfo>
#include <QDBusInterface>
#include <QNetworkReply>
#include <QDBusReply>
#include <QJsonObject>
#include <QJsonArray>

static QString g_captexec = "/opt/cndrvcups-capt/addprinter.sh";

static QString probeDevName(const QString &serial)
{
    for (int i = 0; i < 10; ++i) {
        QString path = QString("/dev/usb/lp%1").arg(i);
        if (!QFile::exists(path))
            continue;

        QProcess proc;
        proc.start("udevadm", QStringList{"info", "-a", path});
        proc.waitForFinished();
        if (proc.exitStatus() == QProcess::NormalExit && !proc.exitCode()) {
            QRegularExpression re("ATTRS{serial}==\"(.*)\"");
            QString line = proc.readLine();
            while (!line.isEmpty()) {
                QRegularExpressionMatch match = re.match(line);
                if (match.hasMatch() && match.captured(1).toLower() == serial)
                    return path;
                line = proc.readLine();
            }
        }
    }

    return QString();
}

static bool isCanonCAPTDrv(const QString &ppd_name)
{
    QRegularExpression re("CNCUPS.*CAPT.*\\.ppd");
    QRegularExpressionMatch match = re.match(ppd_name);
    return match.hasMatch();
}

static bool isHplipDrv(const QString &ppd_name)
{
    return (ppd_name.startsWith("drv:///hpcups.drv") ||
            ppd_name.startsWith("drv:///hpijs.drv") ||
            ppd_name.startsWith("lsb/usr/hplip/") ||
            ppd_name.startsWith("hplip:") ||
            ppd_name.startsWith("hplip-data:") ||
            ppd_name.startsWith("hpijs-ppds:"));
}

static QDBusInterface* getPackageInterface()
{
    static QDBusInterface interface {
        "com.deepin.lastore",
        "/com/deepin/lastore",
        "com.deepin.lastore.Manager",
        QDBusConnection::systemBus()
    };
    return &interface;
}

InstallInterface::InstallInterface(QObject *parent)
    :QObject (parent),
      m_bQuit(false)
{}

void InstallInterface::setPackages(const QStringList &packages)
{
    m_packages = packages;
}

QString InstallInterface::getErrorString()
{
    return m_strErr;
}

void InstallInterface::startInstallPackages()
{
    qInfo() << "install packages:" << m_packages;

    //不用检查包是否已经安装，DBus的接口已经判断了包是否已经安装，并且可以进行升级
    QDBusInterface *interface = getPackageInterface();
    for (auto package : m_packages) {
        QDBusReply<bool> installable = interface->call("PackageInstallable", package);

        if (!installable.isValid() || !installable) {
            m_strErr = package  + tr("is invalid");
            emit signalStatus(TStat_Fail);
            return;
        }
    }

    QDBusReply<QDBusObjectPath> objPath = interface->call("InstallPackage", "", m_packages.join(" "));
    if (objPath.isValid()) {
        m_jobPath = objPath.value().path();
        if (QDBusConnection::systemBus().connect("com.deepin.lastore",
                    m_jobPath,
                    "org.freedesktop.DBus.Properties",
                    "PropertiesChanged",
                    this, SLOT(propertyChanged(QDBusMessage)))) {
            qDebug() << "Start install " << m_packages;
            return;
        }
    }

    m_strErr = tr("Failed to install the driver by calling dbus interface");
    emit signalStatus(TStat_Fail);
}

void InstallInterface::stop()
{
    m_bQuit = true;

    QDBusConnection::systemBus().disconnect("com.deepin.lastore",
            m_jobPath,
            "org.freedesktop.DBus.Properties",
            "PropertiesChanged",
            this, SLOT(propertyChanged(QDBusMessage)));
}

void InstallInterface::propertyChanged(const QDBusMessage &msg)
{
    QList<QVariant> arguments = msg.arguments();

    if (m_bQuit) return;

    if (3 == arguments.count()) {
        QString strType, strStatus;
        QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments[1].value<QDBusArgument>());
        for (auto prop  = changedProps.begin(); prop != changedProps.end(); ++prop) {
            QString key = prop.key();
            if (key == "Type")
                m_strType = prop.value().toString();
            else if (key == "Status")
                m_strStatus = prop.value().toString();
        }

        qDebug() << m_strType << m_strStatus;
        if (m_strType == "install" && m_strStatus == "succeed") {
            emit signalStatus(TStat_Suc);
            goto  done;
        }

        return;
    }

    m_strErr = tr("Failed to install %1").arg(m_packages.join(" "));
    emit signalStatus(TStat_Fail);

done:
    qDebug() << "Disconnect com.deepin.lastore PropertiesChanged";
    QDBusConnection::systemBus().disconnect("com.deepin.lastore",
            m_jobPath,
            "org.freedesktop.DBus.Properties",
            "PropertiesChanged",
            this, SLOT(propertyChanged(QDBusMessage)));

    return;
}

InstallDriver::InstallDriver(const QMap<QString, QVariant> &solution, QObject* parent)
    :InstallInterface(parent)
{
    m_solution = solution;

}

void InstallDriver::doWork()
{
    qDebug() << "Search driver for" << m_solution;
    m_serverInterface = g_printerServer->searchDriver(m_solution[SD_KEY_sid].toInt());

    if (m_serverInterface) {
        connect(m_serverInterface, &PrinterServerInterface::signalDone, this, &InstallDriver::slotServerDone);
        m_serverInterface->postToServer();
    } else {
        emit signalStatus(TStat_Fail);
    }
}

void InstallDriver::stop()
{
    InstallInterface::stop();

    if (m_serverInterface) {
        disconnect(m_serverInterface, &PrinterServerInterface::signalDone, this, &InstallDriver::slotServerDone);
    }
}

void InstallDriver::slotServerDone(int iCode, const QByteArray &result)
{
    if (m_bQuit) return;

    if (iCode != QNetworkReply::NoError) {
        m_strErr = tr("Failed to find the driver solution:%1, error: %2")
                .arg(m_solution[SD_KEY_sid].toInt()).arg(iCode);
        qWarning() << "Request " << m_solution[SD_KEY_sid] << "failed:" << iCode;
        emit signalStatus(TStat_Fail);
        return;
    }

    QJsonParseError err;
    QJsonDocument doc =  QJsonDocument::fromJson(result, &err);
    if (doc.isNull()) {
        m_strErr = tr("The solution is invalid");
        qWarning() << "Request " << m_solution[SD_KEY_sid] << " return nullptr";
        emit signalStatus(TStat_Fail);
        return;
    }

    qDebug() << doc.toJson();
    QJsonObject obj = doc.object();
    QJsonArray package_array = obj[SD_KEY_package].toArray();

    for (auto it : package_array) {
        m_packages << it.toString();
    }

    startInstallPackages();
}

AddCanonCAPTPrinter::AddCanonCAPTPrinter(const TDeviceInfo &printer, const QMap<QString, QVariant> &solution, const QString &uri, QObject *parent)
    :AddPrinterTask(printer, solution, uri, parent)
{
    connect(&m_proc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotProcessFinished(int,QProcess::ExitStatus)));
}

void AddCanonCAPTPrinter::stop()
{
    AddPrinterTask::stop();

    disconnect(&m_proc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotProcessFinished(int,QProcess::ExitStatus)));
    m_proc.kill();
}

int AddCanonCAPTPrinter::addPrinter()
{
    QString ppd_name;
    ppd_name = m_solution[CUPS_PPD_NAME].toString();

    qInfo() << m_printer.strName << ppd_name << m_uri;

    if (!QFile::exists(g_captexec)) {
        qWarning() << g_captexec << "not found";
        emit signalStatus(TStat_Fail);
        return -1;
    }

    if (m_bQuit) return 0;

    m_proc.start("pkexec", QStringList{g_captexec, m_printer.strName, ppd_name, m_uri});

    return 0;
}

void AddCanonCAPTPrinter::slotProcessFinished(int iCode, QProcess::ExitStatus exitStatus)
{
    qInfo() << iCode << exitStatus;
    if (exitStatus == QProcess::NormalExit && 0 == iCode) {
        emit signalStatus(TStat_Suc);
    } else {
        m_strErr = m_proc.readAllStandardError();
        emit signalStatus(TStat_Fail);
    }
}

DefaultAddPrinter::DefaultAddPrinter(const TDeviceInfo &printer, const QMap<QString, QVariant> &solution, const QString &uri, QObject *parent)
    :AddPrinterTask(printer, solution, uri, parent)
{
    connect(&m_proc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotProcessFinished(int,QProcess::ExitStatus)));
}

void DefaultAddPrinter::stop()
{
    AddPrinterTask::stop();

    disconnect(&m_proc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotProcessFinished(int,QProcess::ExitStatus)));
    m_proc.kill();
}

int DefaultAddPrinter::addPrinter()
{
    QStringList args;
    QString ppd_name;
    int driverType = m_solution[SD_KEY_from].toInt();
    ppd_name = m_solution[CUPS_PPD_NAME].toString();

    if (m_bQuit) return 0;

    args << "-p" << m_printer.strName << "-E" << "-v" << m_uri;
    if (PPDFrom_EveryWhere == driverType)
        args << "-m" << "everywhere";
    else if (driverType == PPDFrom_File)
        args << "-P" << ppd_name;
    else
        args << "-m" << ppd_name;

    if (!m_printer.strInfo.isEmpty())
        args << "-D" << m_printer.strInfo;
    if (!m_printer.strLocation.isEmpty())
        args << "-L" << m_printer.strLocation;

    qInfo() << args ;
    m_proc.start("/usr/sbin/lpadmin", args);

    return 0;
}

void DefaultAddPrinter::slotProcessFinished(int iCode, QProcess::ExitStatus exitStatus)
{
    qInfo() << iCode << exitStatus;
    if (exitStatus == QProcess::NormalExit && 0 == iCode) {
        emit signalStatus(TStat_Suc);
    } else {
        int index;
        m_strErr = m_proc.readAllStandardError();
        index = m_strErr.indexOf("\n")+1;
        if (index < m_strErr.length())
            m_strErr = m_strErr.mid(index);
        emit signalStatus(TStat_Fail);
    }
}

AddPrinterTask::AddPrinterTask(const TDeviceInfo &printer, const QMap<QString, QVariant> &solution, const QString &uri, QObject *parent)
    :QObject(parent),
      m_installDepends(nullptr),
      m_installDriver(nullptr),
      m_bQuit(false)
{
    m_printer = printer;
    m_solution = solution;
    m_uri = uri;
}

int AddPrinterTask::isUriAndDriverMatched()
{
    bool is_hplip;
    QString ppd_name;

    //不是直连打印机不检查驱动是否匹配
    if (m_printer.strClass.compare("direct"))
        return 0;

    ppd_name = m_solution[CUPS_PPD_NAME].toString();
    is_hplip = isHplipDrv(ppd_name);

    m_uri.clear();
    //如果是惠普打印机，匹配驱动和uri，hplip的驱动需要用hp:开头的uri添加
    for (auto uri : m_printer.uriList) {
        bool ishpuri = uri.startsWith("hp:");
        if (ishpuri == is_hplip) {
            m_uri = uri;
            break;
        }
    }
    if (m_uri.isEmpty()){
        m_strErr = tr("URI and driver do not match.");
        if (is_hplip) {
            m_strErr += tr("Install hplip first and restart the app to install the driver again.");
        } else {
            m_strErr += tr("Please select an hplip driver and try again.");
        }
        emit signalStatus(TStat_Fail);
        return -1;
    }

    return 0;
}

int AddPrinterTask::checkUriAndDriver()
{
    QString ppd_name;

    if (m_printer.uriList.isEmpty()) {
        m_strErr = tr("URI can't be empty");

        emit signalStatus(TStat_Fail);
        return -1;
    }

    ppd_name = m_solution[CUPS_PPD_NAME].toString();
    if (m_solution[SD_KEY_from].toInt() == PPDFrom_File && !QFile::exists(ppd_name)) {
        m_strErr = ppd_name + tr(" not found");
        emit signalStatus(TStat_Fail);
    }

    return isUriAndDriverMatched();
}

int AddPrinterTask::doWork()
{
    qInfo() << m_printer.uriList << m_solution[CUPS_PPD_NAME].toString();

    if (0 != checkUriAndDriver())
        return -1;

    fillPrinterInfo();

    if (0 == fixDriverDepends()) {
        if (0 ==installDriver()) {
            addPrinter();
        }
    }

    return 0;
}

QString AddPrinterTask::getErrorMassge()
{
    return m_strErr;
}

TDeviceInfo AddPrinterTask::getPrinterInfo()
{
    return m_printer;
}

QMap<QString, QVariant> AddPrinterTask::getDriverInfo()
{
    return m_solution;
}

int AddPrinterTask::fixDriverDepends()
{
    if (m_solution[SD_KEY_from].toInt() == PPDFrom_EveryWhere)
        return 0;

    QString ppd_name = m_solution[CUPS_PPD_NAME].toString();
    QStringList depends = g_driverManager->getDriverDepends(ppd_name.toUtf8().data());
    if (!depends.isEmpty()) {
        m_installDepends = new InstallInterface(this);
        m_installDepends->setPackages(depends);
        connect(m_installDepends, &InstallInterface::signalStatus, this, &AddPrinterTask::slotDependsStatus);
        m_installDepends->startInstallPackages();

        return 1;
    }

    return 0;
}

int AddPrinterTask::installDriver()
{
    if (m_solution[SD_KEY_from].toInt() == PPDFrom_Server) {
        m_installDriver = new InstallDriver(m_solution, this);
        connect(m_installDriver, &InstallDriver::signalStatus, this, &AddPrinterTask::slotInstallStatus);
        m_installDriver->doWork();

        return 1;
    }

    return 0;
}

void AddPrinterTask::fillPrinterInfo()
{
    m_printer.strName = g_addPrinterFactoty->defaultPrinterName(m_printer, m_solution);

    if (m_printer.strLocation.isEmpty()) {
        QString strUri = m_printer.uriList[0];
        QString strHost = getHostFromUri(strUri);

        if (strHost.isEmpty()) {
           if (strUri.startsWith("hp") || strUri.startsWith("usb")) {
               strHost = tr("Direct attached Device");
           } else if (strUri.startsWith("file")) {
               strHost = tr("File device");
           }
        }

        m_printer.strLocation = strHost;
    }

    if (m_printer.strInfo.isEmpty()) {
        QString strModel = m_solution[CUPS_PPD_MODEL].toString();

        m_printer.strInfo = strModel.isEmpty()?m_solution[CUPS_PPD_MAKE_MODEL].toString() : strModel;
    }
}

void AddPrinterTask::stop()
{
    m_bQuit = true;

    if (m_installDriver) {
        disconnect(m_installDriver, &InstallDriver::signalStatus, this, &AddPrinterTask::slotInstallStatus);
        m_installDriver->stop();
    }
}

QStringList GetSystemPrinterNames()
{
    QStringList printerNames;
    map<string, map<string, string>> printers;
    map<string, map<string, string>>::iterator itmap;

    try {
        printers = g_cupsConnection->getPrinters();

        for(itmap=printers.begin();itmap != printers.end();itmap++){
            printerNames << STQ(itmap->first);
        }
    }catch(const std::exception &ex) {
        qWarning() << "Got execpt: " << QString::fromUtf8(ex.what());
    };

    qDebug() << printerNames;
    return printerNames;
}

void AddPrinterTask::slotDependsStatus(int iStatus)
{
    if(m_bQuit) return;

    if (TStat_Suc == iStatus) {
        if (0 ==installDriver()) {
            addPrinter();
        }
    } else {
        m_strErr = m_installDepends->getErrorString();
        emit signalStatus(iStatus);
    }
}

void AddPrinterTask::slotInstallStatus(int iStatus)
{
    if(m_bQuit) return;

    if (TStat_Suc == iStatus) {
        addPrinter();
    } else {
        m_strErr = m_installDriver->getErrorString();
        emit signalStatus(iStatus);
    }
}


AddPrinterFactory* AddPrinterFactory::getInstance()
{
    static AddPrinterFactory* instance = nullptr;
    if (!instance)
        instance = new AddPrinterFactory();

    return instance;
}

AddPrinterTask* AddPrinterFactory::createAddPrinterTask(const TDeviceInfo &printer, const QMap<QString, QVariant> &solution)
{
    QString ppd_name;
    QString device_uri = printer.uriList[0];

    ppd_name = solution[CUPS_PPD_NAME].toString();
    if (ppd_name.isEmpty())
        return nullptr;

    qInfo() << "add printer task:" << device_uri << solution[SD_KEY_from] << ppd_name;
    /* Canon CAPT local printer must use ccp backend */
    if (isCanonCAPTDrv(ppd_name) && !printer.strClass.compare("direct")) {
        device_uri = probeDevName(printer.serial);
        if (!device_uri.isEmpty()) {
            return new AddCanonCAPTPrinter(printer, solution, device_uri);
        }
    }

    return new DefaultAddPrinter(printer, solution, device_uri);
}

QString AddPrinterFactory::defaultPrinterName(const TDeviceInfo &printer, const QMap<QString, QVariant> &solution)
{
    QString strDefaultName;
    QString strName = printer.strName;
    QStringList installedPrinters = GetSystemPrinterNames();

    if(strName.isEmpty()) {
        QString strMM = printer.strMakeAndModel.isEmpty()?solution.value(CUPS_PPD_MAKE_MODEL).toString():printer.strMakeAndModel;
        QString strMake = solution.value(CUPS_PPD_MAKE).toString();

        if (strMM.isEmpty() && !strMake.isEmpty()) {
            strMM = strMake  + " " + solution.value(CUPS_PPD_MODEL).toString();
        }

        //EveryWhere 不用makeandmodel作为名称，因为会包含中文
        if (strMM.isEmpty() || PPDFrom_EveryWhere == solution[SD_KEY_from].toInt()) {
            strName = printer.uriList[0].split("/").last();
        } else {
            QString strModel;
            ppdMakeModelSplit(strMM, strMake, strModel);
            strName = strMake + " " + strModel;
        }
    }

    //打印机名字不能超过128个字符
    if (strName.length() >=  128) {
        strName = strName.left(120);
    }
    strName.replace(QRegularExpression("[^\\w-]"), " ");
    QStringList list = strName.split(" ");
    list.removeAll("");
    strName = list.join(" ");
    strName.replace(" ", "-");
    strDefaultName = strName;

    //保证和已安装的打印机名字不重复
    int i = 1;
    while (installedPrinters.contains(strDefaultName)) {
        strDefaultName = strName + "-" + QString::number(i++);
    }

    return strDefaultName;
}

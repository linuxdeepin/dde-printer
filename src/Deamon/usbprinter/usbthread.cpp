// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "usbthread.h"
#include "cupsconnectionfactory.h"
#include "cupsattrnames.h"
#include "qtconvert.h"
#include "addprinter.h"
#include "zdrivermanager.h"
#include "printerservice.h"


#include <DApplication>
#include <DNotifySender>

#include <QDBusPendingReply>
#include <QDBusConnection>
#include <QProcess>

#include <map>
#include <string>

#include <cups/cups.h>

using namespace std;
DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

static bool isUSBPrinterDevice(const struct libusb_interface_descriptor *interface)
{
    if (!interface)
        return false;
    if ((interface->bInterfaceClass == LIBUSB_CLASS_PRINTER) && (interface->bInterfaceSubClass == 1)) {
        return true;
    }
    return false;
}

static bool isUSBPrinterDevice(const struct libusb_interface *interface)
{
    bool ret = false;
    if (!interface)
        return ret;
    int i;
    for (i = 0; i < interface->num_altsetting; i++) {
        ret = isUSBPrinterDevice(&interface->altsetting[i]);
        if (ret)
            break;
    }

    return ret;
}

static bool isUSBPrinterDevice(const struct libusb_config_descriptor *config)
{
    bool ret = false;
    if (!config)
        return ret;
    int i;
    for (i = 0; i < config->bNumInterfaces; i++) {
        ret = isUSBPrinterDevice(&config->interface[i]);
        if (ret)
            break;
    }

    return ret;
}


/*判断打印机是否已经添加过*/
static bool isArrivedUSBPrinterAdded(const map<string, string> &infoMap, TDeviceInfo &deviceInfo)
{

    if (infoMap.count("Manufacturer") == 0 || infoMap.count("Product") == 0 || infoMap.count("SerialNumber") == 0)
        return true;
    /*同一台惠普打印机可能会存在两个uri，需要都查找出来，作为打印机是否添加判断添加，避免重复添加*/
    vector<string> inSechemes{"usb", "hp"};
    vector<string> exSechemes;
    map<string, map<string, string> > devsMap;
    std::unique_ptr<Connection> conPtr;
    try {
        conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr)
            devsMap = conPtr->getDevices(&exSechemes, &inSechemes, 0, CUPS_TIMEOUT_DEFAULT);
    } catch (const std::exception &ex) {
        qWarning() << "Got execpt: " << QString::fromUtf8(ex.what());
        return true;

    }
    QStringList uriList;
    map<string, string> devices;
    /*从usb后端发现的设备中查找对应序列号的设备*/
    for (auto itmap = devsMap.begin(); itmap != devsMap.end(); itmap++) {
        QString uri = QString::fromStdString(itmap->first);
        if (uri.contains(QString::fromStdString(infoMap.at("SerialNumber"))) && (uri.startsWith("usb:") || uri.startsWith("hp:"))) {
            uriList << uri;
            devices = itmap->second;
        }
    }
    if (uriList.isEmpty() || devices.size() == 0) {
        qWarning() << QString("device not found from cups,product:%1");
        return true;
    }
    /*从cups返回的已经添加的打印机中查找是否存在对应的uri*/
    map<string, map<string, string>> printersMap;
    try {
        printersMap = conPtr->getPrinters();
        for (auto iter = printersMap.begin(); iter != printersMap.end(); iter++) {
            map<string, string> mapProperty = iter->second;
            if (mapProperty.count(CUPS_DEV_URI) == 0)
                continue;
            QString strValue = attrValueToQString(mapProperty[CUPS_DEV_URI]);

            if (uriList.contains(strValue)) {
                return true;
            }
        }
    } catch (const std::runtime_error &e) {
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
        return true;
    }

    /* 清空历史信息 */
    deviceInfo.uriList.clear();
    /*创建设备，用于后续查询驱动和添加打印机*/
    deviceInfo.uriList << uriList;
    deviceInfo.strClass = attrValueToQString(devices[CUPS_DEV_CLASS]);
    deviceInfo.strInfo = attrValueToQString(devices[CUPS_DEV_INFO]);
    deviceInfo.strMakeAndModel = attrValueToQString(devices[CUPS_DEV_MAKE_MODE]);
    deviceInfo.strDeviceId = attrValueToQString(devices[CUPS_DEV_ID]);
    deviceInfo.strLocation = attrValueToQString(devices[CUPS_DEV_LOCATION]);
    deviceInfo.iType = InfoFrom_Detect;
    deviceInfo.strName = deviceInfo.strInfo;
    deviceInfo.serial = QString::fromStdString(infoMap.at("SerialNumber"));

    return false;
}



static map<string, string>  getInfomationFromDescription(libusb_device_handle *pHandle, const libusb_device_descriptor &desc)
{
    map<string, string> infoMap;
    if (!pHandle)
        return infoMap;
    unsigned char ustring[256];
    memset(ustring, 0, sizeof(ustring));
    int ret = 0;
    if (desc.iManufacturer) {

        ret = libusb_get_string_descriptor_ascii(pHandle, desc.iManufacturer, ustring, sizeof(ustring));
        if (ret > 0) {
            qInfo() << QString("Manufacturer:%1").arg((char *)ustring);
            infoMap.insert(make_pair<string, string>("Manufacturer", (char *)ustring));
        }

    }
    memset(ustring, 0, sizeof(ustring));
    if (desc.iProduct) {
        ret = libusb_get_string_descriptor_ascii(pHandle, desc.iProduct, ustring, sizeof(ustring));
        if (ret > 0) {
            qInfo() << QString("Product:%1").arg((char *)ustring);
            infoMap.insert(make_pair<string, string>("Product", (char *)ustring));
        }

    }
    memset(ustring, 0, sizeof(ustring));
    if (desc.iSerialNumber) {
        ret = libusb_get_string_descriptor_ascii(pHandle, desc.iSerialNumber, ustring, sizeof(ustring));
        if (ret > 0) {
            infoMap.insert(make_pair<string, string>("SerialNumber", (char *)ustring));
        }

    }
    return infoMap;
}


USBThread::USBThread(QObject *parent)
    : QThread(parent)
    , needExit(false)
    , m_currentUSBDevice(nullptr)
{
    /*槽函数处于主线程执行*/
    bool ret = connect(this, &USBThread:: newUSBDeviceArrived, this, &USBThread::processArrivedUSBDevice);
    if (!ret)
        qWarning() << "connect to newUSBDeviceArrived:" << ret;
    /*绑定系统通知的点击调用*/
    QDBusConnection conn = QDBusConnection::sessionBus();
    ret = conn.connect("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", "ActionInvoked", this, SLOT(notificationActionInvoked(uint, const QString &)));
    if (!ret)
        qWarning() << "connect to org.freedesktop.Notifications:" << ret;

}

USBThread::~USBThread()
{

}

void USBThread::run()
{
    qInfo() << "USB monitor running...";
    libusb_hotplug_callback_handle usb_arrived_handle;
    libusb_context *ctx;
    int rc = 0;
    do {

        rc = libusb_init(&ctx);
        if (rc < 0) {
            return;
        }
        rc = libusb_hotplug_register_callback(ctx, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED,
                                              LIBUSB_HOTPLUG_NO_FLAGS, LIBUSB_HOTPLUG_MATCH_ANY, LIBUSB_HOTPLUG_MATCH_ANY,
                                              LIBUSB_HOTPLUG_MATCH_ANY, static_usb_arrived_callback, this, &usb_arrived_handle);
        if (LIBUSB_SUCCESS != rc) {
            qWarning() << "Error to register usb arrived callback";
            break;
        }

        while (!needExit) {
            libusb_handle_events_completed(ctx, nullptr);
            sleep(1);
        }

        libusb_hotplug_deregister_callback(ctx, usb_arrived_handle);

    } while (false);
    libusb_exit(ctx);
    qInfo() << "USB monitor exit";
}

bool USBThread::addArrivedUSBPrinter()
{
    DriverSearcher *pDriverSearcher = static_cast<DriverSearcher *>(sender());
    if (pDriverSearcher) {
        QList<QMap<QString, QVariant>> drivers = pDriverSearcher->getDrivers();
        pDriverSearcher->deleteLater();
        foreach (auto driver, drivers) {
            if (driver.contains(SD_KEY_excat) && driver[SD_KEY_excat].toBool()) {
                m_configingPrinterName = g_addPrinterFactoty->defaultPrinterName(m_deviceInfo, driver);
                /*查找到精确匹配驱动发送系统通知，不需要响应*/
                QString strReason = tr("Configuring the printer %1 ...").arg(m_configingPrinterName);
                DUtil::DNotifySender(qApp->productName())
                .appName("dde-printer")
                .appIcon(":/images/dde-printer.svg")
                .appBody(strReason)
                .replaceId(0)
                .timeOut(3000)
                .actions(QStringList() << "default")
                .call();

                emit deviceStatusChanged(m_configingPrinterName, 0);
                QThread::sleep(3);
                AddPrinterTask *task = g_addPrinterFactoty->createAddPrinterTask(m_deviceInfo, driver);
                connect(task, &AddPrinterTask::signalStatus, this, &USBThread::addingJobFinished);
                task->doWork();

                return true;
            }
        }
        qWarning() << QString("could not find avaliable dirvers");
    }
    nextConfiguration();
    return false;
}

void USBThread::notificationActionInvoked(uint id, const QString &msg)
{
    Q_UNUSED(msg)
    if (m_pendingNotificationsMap.contains(id)) {
        QProcess process;
        QString cmd = "dde-printer";
        QStringList args;
        args << "-p" << m_pendingNotificationsMap.value(id);

        if (!process.startDetached(cmd, args)) {
            qWarning() << QString("showMainWindow failed because %1").arg(process.errorString());
        }
        m_pendingNotificationsMap.remove(id);
    }
}

void USBThread::addingJobFinished(int status)
{
    QString strReason = "";
    if (status == TStat_Suc) {
        emit deviceStatusChanged(m_configingPrinterName, 1);
        strReason = tr("Configuration successful. Click to view %1.").arg(m_deviceInfo.strName);
        qInfo() << QString("add printer(%1) success").arg(m_deviceInfo.strName);
    } else {
        emit deviceStatusChanged(m_configingPrinterName, 2);
        strReason = tr("Configuration failed. Click to add the printer %1.").arg(m_deviceInfo.strName);
        qWarning() << QString("add printer(%1) failed").arg(m_deviceInfo.strName);
    }

    QDBusPendingReply<unsigned int> reply = DUtil::DNotifySender(qApp->productName())
                                            .appName("dde-printer")
                                            .appIcon(":/images/dde-printer.svg")
                                            .appBody(strReason)
                                            .replaceId(0)
                                            .timeOut(3000)
                                            .actions(QStringList() << "default")
                                            .call();

    reply.waitForFinished();
    uint ret = 0;
    if (reply.isValid())
        ret = reply.value();
    if (ret > 0)
        m_pendingNotificationsMap.insert(ret, m_configingPrinterName);
    if (sender()) {
        delete sender();
    }
    m_configingPrinterName = "";
    nextConfiguration();
}

void USBThread::getDriver()
{
    DriverSearcher *pDriverSearcher = g_driverManager->createSearcher(m_deviceInfo);
    /*考虑到自动添加打印机每次需要刷新本地驱动非常耗时，而且后续会对接驱动平台，数据更加完善，所以自动添加打印机不进行本地驱动匹配*/
    pDriverSearcher->setMatchLocalDriver(false);
    connect(pDriverSearcher, &DriverSearcher::signalDone, this, &USBThread::addArrivedUSBPrinter);
    pDriverSearcher->startSearch();

}

/*清理当前libusb_device 并继续添加打印机*/
void USBThread::nextConfiguration()
{
    if (m_usbDeviceList.contains(m_currentUSBDevice)) {
        m_usbDeviceList.removeOne(m_currentUSBDevice);
        m_currentUSBDevice = nullptr;
    }
    if (m_usbDeviceList.count() > 0) {
        emit newUSBDeviceArrived();
    }
}

void USBThread::processArrivedUSBDevice()
{
    if ((m_usbDeviceList.count() <= 0) || m_currentUSBDevice) {
        qInfo() << "usbdevice not exist or configing";
        return;
    }
    m_currentUSBDevice = m_usbDeviceList.first();
    if (m_currentUSBDevice == nullptr) {
        qInfo() << "m_currentUSBDevice is nullptr";
        return;
    }

    libusb_device_handle *pHandle = nullptr;
    struct libusb_device_descriptor desc;
    int ret = libusb_get_device_descriptor(m_currentUSBDevice, &desc);
    if (ret < 0) {
        qWarning() << "failed to get device descriptor";
        nextConfiguration();
        return;
    }

    qInfo() << QString("Device vendor:%1 product:%2").arg(desc.idVendor).arg(desc.idProduct);

    if (!pHandle)
        libusb_open(m_currentUSBDevice, &pHandle);

    bool isUSBPrinter = false;
    for (uint8_t i = 0; i < desc.bNumConfigurations; i++) {
        struct libusb_config_descriptor *config = nullptr;

        ret = libusb_get_config_descriptor(m_currentUSBDevice, i, &config);
        if (LIBUSB_SUCCESS != ret) {
            qWarning() << "Couldn't retrieve descriptors";
            continue;
        }

        isUSBPrinter = isUSBPrinterDevice(config);

        libusb_free_config_descriptor(config);
        if (isUSBPrinter)
            break;
    }

    bool isAdded = true;
    if (isUSBPrinter) {
        map<string, string> infoMap = getInfomationFromDescription(pHandle, desc);

        isAdded = isArrivedUSBPrinterAdded(infoMap, m_deviceInfo);
        if (!isAdded) {
            qInfo() << "begin to parse printer driver";
            getDriver();
        } else {
            qInfo() << "The printer has been added";

        }
    }

    if (pHandle)
        libusb_close(pHandle);

    if (isAdded) {
        nextConfiguration();
    }

}

int LIBUSB_CALL USBThread::static_usb_arrived_callback(struct libusb_context *ctx, struct libusb_device *dev,
                                                       libusb_hotplug_event event, void *userdata)
{
    if (userdata)
        return reinterpret_cast<USBThread *>(userdata)->usb_arrived_callback(ctx, dev, event);
    else {
        qWarning() << "userdata is null";
        return -1;
    }
}

int USBThread::usb_arrived_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event)
{
    /*
     * 1.先判断设备类型是否是打印机 bInterfaceClass =7 bInterfaceSubClass=1
     * 2.获取serial
     * 3.调用cups http api 获取 usb类型打印机
     * 4.从列表中找到和serial对应的打印机
     * 5.判断该打印机是否已经被添加过（uri）
     * 6.自动添加打印机并给出系统通知
    */
    /*
     * NOTE:回调函数结束后才能开始IO操作，不然会返回LIBUSB_ERROR_BUSY错误状态
     * 考虑到多个设备同时插入的情况，需要做一个队列排队添加打印机。存在跨线程访问数据的情况，使用锁同步。
     * 当m_currentUSBDevice为空时触发添加流程，添加完m_currentUSBDevice置为空，通过信号触发新的添加流程
    */
    Q_UNUSED(ctx)
    Q_UNUSED(event)
    if (!m_usbDeviceList.contains(dev))
        m_usbDeviceList.push_back(dev);

    if (m_usbDeviceList.count() > 0 && (m_currentUSBDevice == nullptr)) {
        emit newUSBDeviceArrived();
    }

    return (dev) ? 0 : -1;
}


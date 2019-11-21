#include "common.h"
#include "config.h"
#include "qtconvert.h"
#include "cupsattrnames.h"
#include "dprintermanager.h"

#include <QVariant>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QProcess>
#include <QDebug>
#include <QUrl>
#include <QStringList>
#include <QDBusInterface>
#include <QDBusReply>
#include <QRegExpValidator>
#include <QDebug>

#include <netdb.h>
extern int h_errno;

QString getPrinterPPD(const char *name)
{
    QString strPPD;

    try {
        strPPD = STQ(g_cupsConnection->getPPD(name));
    }catch(const std::exception &ex) {
        qWarning() << "Got execpt: " << QString::fromUtf8(ex.what());
        return QString();
    };

    return strPPD;
}

QString getPrinterNameFromUri(const QString &uri)
{
    int index = uri.indexOf("printers/");
    return uri.right(uri.length()-index-9);
}

QString getPrinterUri(const char *name)
{
    vector<string> requestList;
    map<string, string> attrs;
    QString strUri;

    try {
        requestList.push_back(CUPS_DEV_URI);
        attrs = g_cupsConnection->getPrinterAttributes(name, nullptr, &requestList);
    }catch(const std::exception &ex) {
        qWarning() << "Got execpt: " << QString::fromUtf8(ex.what());
        return QString();
    };

    strUri = attrValueToQString(attrs[CUPS_DEV_URI]);

    return strUri;
}

QString getHostFromUri(const QString &strUri)
{
    QUrl url(strUri);

    //fix for hp:/net/HP_Color_LaserJet_Pro_M252dw?ip=10.0.12.6
    QRegularExpression re("ip=(((?:(?:25[0-5]|2[0-4]\\d|((1\\d{2})|([1-9]?\\d)))\\.){3}(?:25[0-5]|2[0-4]\\d|((1\\d{2})|([1-9]?\\d)))))");
    QRegularExpressionMatch match = re.match(strUri);
    if (match.hasMatch()) {
        return match.captured(1);
    }

    //排除格式不对的
    if (strUri.indexOf("://") == -1)
        return QString();

    //smb格式uri：smb://[username:password@][workgroup/]server/printer
    if (strUri.startsWith("smb://")) {
        QStringList strlist = strUri.split("/");
        QString str = strlist.count()>3 ? strlist[strlist.count()-2] : strlist[strlist.count()-1];
        strlist = str.split("@");
        return strlist.last();
    }

    //dnssd格式uri: dnssd://printername @ host.*.*.local/*
    if (strUri.startsWith("dnssd://")) {
        QStringList strlist = QUrl::fromPercentEncoding(strUri.toUtf8()).split("/");
        strlist = strlist[2].split(" ");
        strlist = strlist.last().split(".");
        return strlist.first() + "." + strlist.last();
    }

    //排除直连设备和文件设备
    if (strUri.startsWith("hp") || strUri.startsWith("usb") || strUri.startsWith("file"))
        return QString();

    return url.host();
}

QString reslovedHost(const QString &strHost)
{
    if (strHost.isEmpty()) return strHost;

    struct hostent *host = gethostbyname(strHost.toUtf8().data());
    if (nullptr == host && HOST_NOT_FOUND == h_errno) {
        return strHost + QObject::tr(" not found, please ask the administrator for help");
    }

    return QString();
}

bool isPackageExists(const QString& package)
{
    QDBusInterface interface {
        "com.deepin.lastore",
        "/com/deepin/lastore",
        "com.deepin.lastore.Manager",
        QDBusConnection::systemBus()
    };

    QDBusReply<bool> isExists = interface.call("PackageExists", package);

    return  isExists.isValid() && isExists;
}

QVariant ipp_attribute_value (ipp_attribute_t *attr, int i)
{
    QVariant val;
//    char unknown[100];
//    int lower, upper;
//    int xres, yres;
//    ipp_res_t units;

    switch (ippGetValueTag(attr)) {
    case IPP_TAG_NAME:
    case IPP_TAG_TEXT:
    case IPP_TAG_KEYWORD:
    case IPP_TAG_URI:
    case IPP_TAG_CHARSET:
    case IPP_TAG_MIMETYPE:
    case IPP_TAG_LANGUAGE:
        qDebug() << QString("Got %1 : %2").arg(UTF8_T_S(ippGetName(attr))).arg(UTF8_T_S(ippGetString(attr, i, nullptr)));
        val = QVariant(UTF8_T_S(ippGetString(attr, i, nullptr)).trimmed());
        break;
    case IPP_TAG_INTEGER:
    case IPP_TAG_ENUM:
        qDebug() << QString("Got %1 : %2").arg(UTF8_T_S(ippGetName(attr))).arg(ippGetInteger(attr, i));
        val = QVariant(ippGetInteger(attr, i));
        break;
    case IPP_TAG_BOOLEAN:
        qDebug() << QString("Got %1 : %2").arg(UTF8_T_S(ippGetName(attr))).arg(ippGetBoolean(attr, i)?"true":"false");
        val = QVariant(ippGetBoolean(attr, i));
        break;
//    case IPP_TAG_RANGE:
//        lower = ippGetRange (attr, i, &upper);
//        val = Py_BuildValue ("(ii)",
//                 lower,
//                 upper);
//        break;
//    case IPP_TAG_NOVALUE:
//        Py_RETURN_NONE;
//        break;
//    // TODO:
//    case IPP_TAG_DATE:
//        val = PyUnicode_FromString ("(IPP_TAG_DATE)");
//        break;
//    case IPP_TAG_RESOLUTION:
//        xres = ippGetResolution(attr, i, &yres, &units);
//        val = Py_BuildValue ("(iii)",
//                 xres,
//                 yres,
//                 units);
//        break;
    default:
//        snprintf (unknown, sizeof (unknown),
//              "(unknown IPP value tag 0x%x)", ippGetValueTag(attr));
//        val = PyUnicode_FromString (unknown);
        break;
    }

    return val;
}

int shellCmd(const QString &cmd, QString& out, QString& strErr, int timeout)
{
    qDebug() << "Start command: " << cmd;
    QProcess proc;
    proc.start(cmd);
    if (proc.waitForFinished(timeout)) {
        out = proc.readAll();
        if (proc.exitCode() != 0 || proc.exitStatus() != QProcess::NormalExit) {
            strErr = QString("err %1, string: %2").arg(proc.exitCode()).arg(QString::fromUtf8(proc.readAllStandardError()));
            return -1;
        }
    }
    else return -2;

    return 0;
}

//将每个单词统一成首字母大写
QString toNormalName(const QString &name)
{
    QString norName;
    QStringList list = name.split(" ");

    if (0 == name.compare("hp", Qt::CaseInsensitive))
        return "HP";
    else if (0 == name.compare("CIAAT", Qt::CaseInsensitive))
        return "CIAAT";
    else if (0 == name.compare("DEC", Qt::CaseInsensitive))
        return "DEC";
    else if (0 == name.compare("IBM", Qt::CaseInsensitive))
        return "IBM";
    else if (0 == name.compare("NEC", Qt::CaseInsensitive))
        return "NEC";
    else if (0 == name.compare("NRG", Qt::CaseInsensitive))
        return "NRG";
    else if (0 == name.compare("PCPI", Qt::CaseInsensitive))
        return "PCPI";
    else if (0 == name.compare("QMS", Qt::CaseInsensitive))
        return "QMS";

    foreach (QString str, list) {
        norName += str.left(1).toUpper();
        norName += str.right(str.length() - 1).toLower();
        norName += " ";
    }
    norName = norName.trimmed();
    return norName;
}

//通过空格分割字符串，在""内的空格不算
QStringList splitStdoutString(const QString &str)
{
    QStringList list;
    QString filed;
    bool quoting = false;

    for (auto it = str.begin(); it != str.end(); ++it) {
        if (*it == QChar(' ')) {
            if (!quoting) {
                list << filed;
                filed.clear();
            } else
                filed += *it;
        } else if (*it == QChar('"')) {
            quoting = !quoting;
        } else {
            if (*it == QChar('\\')) {
                auto next = it + 1;
                if (next != str.end() &&
                        (*next == QChar('\\') || *next == QChar('"')))
                    ++it;
            }
            filed += *it;
        }
    }

    if (!filed.isEmpty())
        list << filed;

    return list;
}

/*# This function normalizes manufacturer and model names for comparing.
# The string is turned to lower case and leading and trailing white
# space is removed. After that each sequence of non-alphanumeric
# characters (including white space) is replaced by a single space and
# also at each change between letters and numbers a single space is added.
# This makes the comparison only done by alphanumeric characters and the
# words formed from them. So mostly two strings which sound the same when
# you pronounce them are considered equal. Printer manufacturers do not
# market two models whose names sound the same but differ only by
# upper/lower case, spaces, dashes, ..., but in printer drivers names can
# be easily supplied with these details of the name written in the wrong
# way, especially if the IEEE-1284 device ID of the printer is not known.
# This way we get a very reliable matching of printer model names.
# Examples:
# - Epson PM-A820 -> epson pm a 820
# - Epson PM A820 -> epson pm a 820
# - HP PhotoSmart C 8100 -> hp photosmart c 8100
# - hp Photosmart C8100  -> hp photosmart c 8100*/
QString normalize(const QString &strin)
{
    QString normalized;
    QString lstrin = strin.trimmed().toLower();
    bool alnumfound = false;
    enum{BLANK, LETTER, DIGIT};
    int lastchar = BLANK;

    if (strin.isEmpty()) return strin;

    foreach (QChar ch, lstrin) {
        if (ch.isLetter())
        {
            if (LETTER != lastchar && alnumfound)
                normalized += " ";
            lastchar = LETTER;
        }
        else if (ch.isDigit())
        {
            if (DIGIT != lastchar && alnumfound)
                normalized += " ";
            lastchar = DIGIT;
        }
        else lastchar = BLANK;

        if (ch.isLetterOrNumber())
        {
            normalized += ch;
            alnumfound = true;
        }
    }

    return normalized;
}

QMap<QString, QString> parseDeviceID(const QString &strId)
{
    QMap<QString, QString> map;
    QStringList list = strId.split(";");
    foreach (QString str, list) {
        QStringList val = str.split(":");
        if(val.count() > 1)
            map.insert(val[0].trimmed(), val[1].trimmed());
    }

    if (!map.contains("MFG") && map.contains("MANUFACTURER"))
        map.insert("MFG", map["MANUFACTURER"]);
    if (!map.contains("MDL") && map.contains("MODEL"))
        map.insert("MDL", map["MODEL"]);
    if (!map.contains("CMD") && map.contains("COMMAND SET"))
        map.insert("CMD", map["COMMAND SET"]);

    return map;
}

static const char* g_replaceMap[][2] = {{"lexmark international", "Lexmark"},
                              {"kyocera mita", "Kyocera"},
                              {"hewlettpackard", "HP"},
                              {"minoltaqms", "minolta"},
                              {"datamaxoneil", "datamax"},
                              {"eastman kodak company", "kodak"},
                              {"fuji xerox", "fuji xerox"},
                              {"konica minolta", "konica minolta"}
                              };
QString replaceMakeName(const QString &make_and_model, int* len)
{
    QString strMake = make_and_model;
    QString strMM = make_and_model.toLower();

    int size = sizeof(g_replaceMap)/sizeof(g_replaceMap[0]);
    for(int i=0;i<size;i++)
    {
        QString str = g_replaceMap[i][0];

        if (strMM.startsWith(str))
        {
            strMake = g_replaceMap[i][1];
            if (len) *len = str.length();

            break;
        }
    }

    return strMake;
}

typedef struct tagMakeRegular{
    QString     strMake;
    QString     strRegular;
}TMakeRegular;

TMakeRegular _MFR_BY_RANGE[] = {
    // Fill in missing manufacturer names based on model name
    {"HP", QString("deskjet"
                      "|dj[ 0-9]?"
                      "|laserjet"
                      "|lj"
                      "|color laserjet"
                      "|color lj"
                      "|designjet"
                      "|officejet"
                      "|oj"
                      "|photosmart"
                      "|ps "
                      "|psc"
                      "|edgeline")},
    {"Epson", QString("stylus|aculaser")},
    {"Apple", QString("stylewriter"
                         "|imagewriter"
                         "|deskwriter"
                         "|laserwriter")},
    {"Canon", QString("pixus"
                         "|pixma"
                         "|selphy"
                         "|imagerunner"
                         "|bj"
                         "|lbp")},
    {"Brother", QString("hl|dcp|mfc")},
    {"Xerox", QString("docuprint"
                         "|docupage"
                         "|phaser"
                         "|workcentre"
                         "|homecentre")},
    {"Lexmark", QString("optra|(:color )?jetprinter")},
    {"KONICA MINOLTA", QString("magicolor"
                                  "|pageworks"
                                  "|pagepro")},
    {"Kyocera", QString("fs-"
                           "|km-"
                           "|taskalfa")},
    {"Ricoh", QString("aficio")},
    {"Oce", QString("varioprint")},
    {"Oki", QString("okipage|microline")}
    };

QString _RE_ignore_suffix = QString(","
                                    "| hpijs"
                                    "| foomatic/"
                                    "| - "
                                    "| w/"
                                    "| postscript"
                                    "| ps"
                                    "| pdf"
                                    "| pxl"
                                    "| zjs"
                                    "| zxs"
                                    "| pcl3"
                                    "| printer"
                                    "|_bt"
                                    "| pcl"
                                    "| ufr ii"
                                    "| br-script"
                                    "| for cups"
                                    "| series"
                                    "| all-in-one"
                                    );

typedef struct tagHPMode{
    QString     strShortName;
    QString     strFullName;
}THPMode;

THPMode _HP_MODEL_BY_NAME[] = {{"dj", "DeskJet"},
                                {"lj", "LaserJet"},
                                {"oj", "OfficeJet"},
                                {"color lj", "Color LaserJet"},
                                {"ps ", "PhotoSmart"}
};


void removeMakeInModel(QString& strMake, QString& strModel)
{
    QString modell = strModel.toLower();
    QString makel = strMake.toLower();
    if (modell.startsWith(makel))
    {
        strModel = strModel.right(strModel.length() - makel.length());
        strModel = strModel.trimmed();
    }
}

static bool remapMakeModel(const QString &strMakeAndModel, QString& strMake, QString& strModel)
{
    int len = 0;
    QString strMM = strMakeAndModel;
    strMM = strMM.replace(QRegularExpression("_|-|\'"), "");
    QString str = replaceMakeName(strMM, &len);
    if (len > 0)
    {
        strMake = str;
        strModel = strMM.right(strMM.length()-len);
        strModel = strModel.trimmed();
        return true;
    }
    return false;
}

void ppdMakeModelSplit(const QString &strMakeAndModel, QString& strMake, QString& strModel)
{
    int len = 0;

    QString Make_And_Model = strMakeAndModel.trimmed();
    QString make_and_model = Make_And_Model.toLower();

    //通过model名字自动填充厂商名
    // If the string starts with a known model name (like "LaserJet") assume
    // that the manufacturer name is missing and add the manufacturer name
    // corresponding to the model name
    len = sizeof(_MFR_BY_RANGE)/sizeof(TMakeRegular);
    for(int i=0;i<len;i++) {
        if(make_and_model.indexOf(QRegularExpression(_MFR_BY_RANGE[i].strRegular)) == 0) {
            strMake = _MFR_BY_RANGE[i].strMake;
            strModel = strMakeAndModel;
            qInfo() << strMake << strModel;
            break;
        }
    }

    //Handle PPDs provided by Turboprint
    if (make_and_model.contains("turboprint")) {
        int start = strMakeAndModel.indexOf(" TurboPrint");
        if(start > -1) {
            start += 12;
            int end = strMakeAndModel.indexOf(" TurboPrint", start);
            if (end > start)
                Make_And_Model = Make_And_Model.mid(start, end-start);
            else
                Make_And_Model = Make_And_Model.left(start-12);

            Make_And_Model = Make_And_Model.trimmed();
            QStringList list = strMakeAndModel.split("_");
            if(list.count() > 1) {
                strMake = list[0];
                list.removeFirst();
                strModel = list.join("_");
            } else {
                strMake = strMakeAndModel;
            }
        }

        strMake.replace(QRegularExpression("(?<=[a-z])(?=[0-9])"), " ");
        strMake.replace(QRegularExpression("(?<=[a-z])(?=[A-Z])"), " ");
        strModel.replace(QRegularExpression("(?<=[a-z])(?=[0-9])"), " ");
        strModel.replace(QRegularExpression("(?<=[a-z])(?=[A-Z])"), " ");
        strModel.replace(" Jet", "Jet");
        strModel.replace("Photo Smart", "PhotoSmart");

        qInfo() << strMakeAndModel << "->" << strMake << strModel;
        // Special handling for two-word manufacturers
    } else if (remapMakeModel(strMakeAndModel, strMake, strModel)) {
        qDebug() << strMakeAndModel << "->" << strMake << strModel;
        // Finally, take the first word as the name of the manufacturer.
    } else {
        QStringList list = strMakeAndModel.split(" ");
        if(list.count() > 1) {
            strMake = list[0];
            list.removeFirst();
            strModel = list.join(" ");
        } else {
            strMake = strMakeAndModel;
        }
    }

    QString makel = strMake.toLower();
    QString modell = strModel.toLower();
    /*# HP and Canon PostScript PPDs give NickNames like:
    # *NickName: "HP LaserJet 4 Plus v2013.111 Postscript (recommended)"
    # *NickName: "Canon MG4100 series Ver.3.90"
    # Find the version number and truncate at that point.  But beware,
    # other model names can legitimately look like version numbers,
    # e.g. Epson PX V500.
    # Truncate only if the version number has only one digit, or a dot
    # with digits before and after.*/
    if (modell.contains(" v")) {
        /*# Look for " v" or " ver." followed by a digit, optionally
        # followed by more digits, a dot, and more digits; and
        # terminated by a space of the end of the line.*/
        int index = modell.indexOf(QRegularExpression(" v(?:er\\.)?\\d(?:\\d*\\.\\d+)?(?: |$)"));
        if (index > 0) {
            strModel = strModel.left(index);
            modell = modell.left(index);
        }
        qDebug() << strMakeAndModel << "->" << strMake << strModel;
    }

    //remove right string contains ignores suffix
    int index = modell.indexOf(QRegularExpression(_RE_ignore_suffix));
    if (index > 0) {
        strModel = strModel.left(index);
        modell = modell.left(index);
        qDebug() << strMakeAndModel << "->" << strMake << strModel;
    }

    if (makel == "hp") {
        len = sizeof(_HP_MODEL_BY_NAME)/sizeof(THPMode);
        for(int i=0;i<len;i++) {
            if (modell.startsWith(_HP_MODEL_BY_NAME[i].strShortName)) {
                strModel = _HP_MODEL_BY_NAME[i].strFullName +
                        strModel.right(strModel.length()-_HP_MODEL_BY_NAME[i].strShortName.length());
                modell = strModel.toLower();
                qDebug() << strMakeAndModel << "->" << strMake << strModel;
                break;
            }
        }
    }

    removeMakeInModel(strMake, strModel);
    strModel = strModel.replace(QRegularExpression("_|-|\'"), " ");
    strModel = strModel.trimmed();
}

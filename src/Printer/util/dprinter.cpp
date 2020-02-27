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
#include "dprinter.h"
#include "dprintermanager.h"

#include <assert.h>
#include <string>
#include <fcntl.h>
#include <sys/stat.h>

#include <QDebug>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QDebug>

DPrinter::DPrinter(Connection *con): DDestination(con)
{
    m_type = DESTTYPE::PRINTER;
    m_bNeedSavePpd = false;
}

bool DPrinter::initPrinterPPD()
{
    bool bRet = false;

    try
    {
        time_t tm = 0;
        string strPPDName = m_pCon->getPPD3(m_strName.toStdString().c_str(), &tm, nullptr);
        m_ppd.load(strPPDName.c_str());
        m_ppd.localize();
        bRet = true;
    }
    catch (const std::runtime_error &e)
    {
        bRet = false;
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
    }

    return bRet;
}

bool DPrinter::isPpdFileBroken()
{
    bool bRet = initPrinterPPD();
    return !bRet;
}

void DPrinter::setPageSize(const QString &strValue)
{
    setOptionValue("PageSize", strValue);
}

QString DPrinter::getPageSize()
{
    return getOptionValue("PageSize");
}

QVector<QMap<QString, QString>> DPrinter::getPageSizeChooses()
{
    return getOptionChooses("PageSize");
}

void DPrinter::setPageRegion(const QString &strValue)
{
    setOptionValue("PageRegion", strValue);
}

QString DPrinter::getPageRegion()
{
    return getOptionValue("PageRegion");
}

QVector<QMap<QString, QString>> DPrinter::getPageRegionChooses()
{
    return getOptionChooses("PageRegion");
}

void DPrinter::setMediaType(const QString& strValue)
{
    setOptionValue("MediaType", strValue);
}

QString DPrinter::getMediaType()
{
    return getOptionValue("MediaType");
}

QVector<QMap<QString, QString>> DPrinter::getMediaTypeChooses()
{
    return getOptionChooses("MediaType");
}

bool DPrinter::canSetColorModel()
{
    bool bRet = false;

    try {
        Attribute attr = m_ppd.findAttr("ColorDevice",nullptr);
        QString strValue = QString::fromStdString(attr.getValue());
        strValue = strValue.trimmed();

        if(strValue == "False")
        {
            bRet = false;
        }
        else
        {
            bRet = true;
        }
    } catch (const std::runtime_error &e) {
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
        bRet = false;
    }

    return bRet;
}

void DPrinter::setColorModel(const QString& strColorMode)
{
    /*
    if (bColor) {
        setOptionValue("ColorModel", "RGB");
    } else {
        setOptionValue("ColorModel", "Gray");
    }
    */
    setOptionValue("ColorModel", strColorMode);
}

QString DPrinter::getColorModel()
{
    QString strColorModel = getOptionValue("ColorModel");

    if(strColorModel.isEmpty())
    {
        strColorModel = getColorAttr();
    }

    return strColorModel;
}

QVector<QMap<QString, QString>> DPrinter::getColorModelChooses()
{
    QVector<QMap<QString, QString>> vecChoose = getOptionChooses("ColorModel");

    if(vecChoose.isEmpty())
    {
        QString strVal = getColorAttr();

        if(!strVal.isEmpty())
        {
            QMap<QString,QString> choose;

            if(strVal == "Gray")
            {
                choose["choice"] = "Gray";
                choose["text"] = "Grayscale";
            }
            else
            {
                choose["choice"] = "RGB";
                choose["text"] = "Color";
            }

            vecChoose.push_back(choose);
        }
    }

    return vecChoose;
}

void DPrinter::setPrintQuality(const QString &strValue)
{
    setOptionValue("OutputMode", strValue);
}

QString DPrinter::getPrintQuality()
{
    return getOptionValue("OutputMode");
}

QVector<QMap<QString, QString>> DPrinter::getPrintQualityChooses()
{
    return getOptionChooses("OutputMode");
}

void DPrinter::setInputSlot(const QString &strValue)
{
    setOptionValue("InputSlot", strValue);
}

QString DPrinter::getInputSlot()
{
    return getOptionValue("InputSlot");
}

QVector<QMap<QString, QString>> DPrinter::getInputSlotChooses()
{
    return getOptionChooses("InputSlot");
}

bool DPrinter::canSetDuplex()
{
    bool bRet = true;
    return bRet;
}

void DPrinter::setDuplex(const QString &strValue)
{
    setOptionValue("Duplex", strValue);
}

QString DPrinter::getDuplex()
{
    return getOptionValue("Duplex");
}

QVector<QMap<QString, QString>> DPrinter::getDuplexChooses()
{
    return getOptionChooses("Duplex");
}

QString DPrinter::getDriverName()
{
    string strLinkPath = m_pCon->getPPD(m_strName.toStdString().c_str());
    return QFile::symLinkTarget(QString::fromStdString(strLinkPath));
}

QString DPrinter::getPrinterMakeAndModel()
{
    QString strMakeAndModel= "";

    try {
        vector<string> requestAttrs;
        requestAttrs.push_back("printer-make-and-model");
        map<string, string> attrs = m_pCon->getPrinterAttributes(m_strName.toStdString().c_str(),
                                                                 nullptr, &requestAttrs);

        for (auto iter = attrs.begin(); iter != attrs.end(); iter++)
        {
            strMakeAndModel = QString::fromStdString(iter->second.data());
            strMakeAndModel = strMakeAndModel.remove(0, 1);
        }
    } catch (const std::runtime_error &e) {
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
        strMakeAndModel.clear();
    }

    return strMakeAndModel;
}

QString DPrinter::getPrinterUri()
{
    QString strUri= "";

    try {
        vector<string> requestAttrs;
        requestAttrs.push_back("device-uri");
        map<string, string> attrs = m_pCon->getPrinterAttributes(m_strName.toStdString().c_str(),
                                                                 nullptr, &requestAttrs);

        for (auto iter = attrs.begin(); iter != attrs.end(); iter++)
        {
            strUri = QString::fromStdString(iter->second.data());
            strUri = strUri.remove(0, 1);
        }
    } catch (const std::runtime_error &e) {
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
        strUri.clear();
    }

    return strUri;
}

void DPrinter::setPrinterUri(const QString& strValue)
{
    try {
        m_pCon->setPrinterDevice(m_strName.toStdString().c_str(), strValue.toStdString().c_str());
    } catch (const std::runtime_error& e) {
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
    }
}

QString DPrinter::getPageOrientation()
{
    QString strOritentation= "";

    try {
        vector<string> requestAttrs;
        requestAttrs.push_back("orientation-requested-default");
        map<string, string> attrs = m_pCon->getPrinterAttributes(m_strName.toStdString().c_str(),
                                                                 nullptr, &requestAttrs);

        for (auto iter = attrs.begin(); iter != attrs.end(); iter++)
        {
            if(0 == iter->first.compare("orientation-requested-default"))
            {
                strOritentation = QString::fromStdString(iter->second.data());

                if(strOritentation.isEmpty())
                {
                    strOritentation = QString::fromStdString("3");
                }
                else
                {
                    strOritentation = strOritentation.remove('i');
                    strOritentation =strOritentation.trimmed();
                }
            }
        }
    } catch (const std::runtime_error &e) {
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
        strOritentation.clear();
    }

    return strOritentation;
}

void DPrinter::setPageOrientationChoose(const QString& strValue)
{
    try
    {
        string strDefault = strValue.toStdString();
        vector<string> Attrs;
        Attrs.push_back(strDefault);
        m_pCon->addPrinterOptionDefault(m_strName.toStdString().c_str(),
                                        "orientation-requested", &Attrs);
    }
    catch (const std::runtime_error& e)
    {
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
    }
}

QVector<QMap<QString, QString>> DPrinter::getPageOrientationChooses()
{
    QVector<QMap<QString, QString>> vecChoose;

    try
    {
        vector<string> requestAttrs;
        requestAttrs.push_back("orientation-requested-supported");
        map<string, string> attrs = m_pCon->getPrinterAttributes(m_strName.toStdString().c_str(),nullptr, &requestAttrs);

        for (auto iter = attrs.begin(); iter != attrs.end(); iter++)
        {
            QMap<QString, QString> choose;

            if(0 == iter->first.compare("orientation-requested-supported"))
            {
                QString strChoose = QString::fromStdString(iter->second);
                //QStringList strChooses = strChoose.split('`');
                QStringList strChooses = strChoose.split('\0');
                DPrinterManager* pManger = DPrinterManager::getInstance();

                for (int i = 0; i < strChooses.size(); i++)
                {
                    QString strTemp = strChooses[i].remove('`');
                    strTemp = strChooses[i].remove('i');
                    strTemp =strTemp.trimmed();

                    if(0 == strTemp.compare("3"))
                    {
                        choose[QString::fromStdString("text")] = pManger->translateLocal("Orientation_Combo", "Portrait (no rotation)");
                        choose[QString::fromStdString("choice")] = strTemp;
                    }
                    else if(0 == strTemp.compare("4"))
                    {
                        choose[QString::fromStdString("text")] = pManger->translateLocal("Orientation_Combo", "Landscape (90 degrees)");
                        choose[QString::fromStdString("choice")] = strTemp;
                    }
                    else if(0 == strTemp.compare("5"))
                    {
                        choose[QString::fromStdString("text")] = pManger->translateLocal("Orientation_Combo", "Reverse landscape (270 degrees)");
                        choose[QString::fromStdString("choice")] = strTemp;
                    }
                    else if(0 == strTemp.compare("6"))
                    {
                        choose[QString::fromStdString("text")] = pManger->translateLocal("Orientation_Combo", "Reverse portrait (180 degrees)");
                        choose[QString::fromStdString("choice")] = strTemp;
                    }
                    else
                    {
                        continue;
                    }

                    vecChoose.push_back(choose);
                }
            }
        }
    }
    catch (const std::runtime_error& e)
    {
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
        vecChoose.clear();
    }

    return vecChoose;
}

QString DPrinter::getPageOutputOrder()
{
    QString strOrder= "";

    try {
        vector<string> requestAttrs;
        requestAttrs.push_back("outputorder-default");
        map<string, string> attrs = m_pCon->getPrinterAttributes(m_strName.toStdString().c_str(),
                                                                 nullptr, &requestAttrs);

        if(0 == attrs.size())
        {
            setPageOutputOrder(QString::fromStdString("normal"));
            strOrder = QString::fromStdString("normal");
        }
        else {
            for (auto iter = attrs.begin(); iter != attrs.end(); iter++)
            {
                if(0 == iter->first.compare("outputorder-default"))
                {
                    strOrder = QString::fromStdString(iter->second.data());

                    if(strOrder.isEmpty())
                    {
                        strOrder = " ";
                    }
                    else
                    {
                        strOrder = strOrder.remove(0, 1);
                        strOrder = strOrder.trimmed();
                    }
                }
            }
        }
    }
    catch (const std::runtime_error &e)
    {
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
        strOrder.clear();
    }

    return strOrder;
}

void DPrinter::setPageOutputOrder(const QString& strValue)
{
    try
    {
        string strDefault = strValue.toStdString();
        vector<string> Attrs;
        Attrs.push_back(strDefault);
        m_pCon->addPrinterOptionDefault(m_strName.toStdString().c_str(),
                                        "outputorder", &Attrs);
    }
    catch (const std::runtime_error& e)
    {
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
    }
}

QVector<QMap<QString, QString>> DPrinter::getPageOutputOrderChooses()
{
    DPrinterManager* pManger = DPrinterManager::getInstance();
    QVector<QMap<QString, QString>> vecChoose;
    //normal,reverse
    QMap<QString,QString> choose;
    choose[QString::fromStdString("text")] = pManger->translateLocal("PrintOrder_Combo", "Normal");
    choose[QString::fromStdString("choice")] = QString::fromStdString("normal");
    vecChoose.push_back(choose);
    choose.clear();
    choose[QString::fromStdString("text")] = pManger->translateLocal("PrintOrder_Combo", "Reverse");
    choose[QString::fromStdString("choice")] = QString::fromStdString("reverse");
    vecChoose.push_back(choose);
    return vecChoose;

    /*
    QVector<QMap<QString, QString>> vecChoose;

    try {
        vector<string> requestAttrs;
        requestAttrs.push_back("outputorder-default");
        map<string, string> attrs = m_pCon->getPrinterAttributes(m_strName.toStdString().c_str(),
                                                                 nullptr, nullptr);

        for (auto iter = attrs.begin(); iter != attrs.end(); iter++) {

        }
    }
    catch (const std::runtime_error& e)
    {
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
    }

    return vecChoose;
    */
}

QString DPrinter::getBindEdgeOption()
{
    return getOptionValue("BindEdge");
}

void DPrinter::setBindEdgetOption(const QString &strValue)
{
    /*
    try
    {
        string strDefault = strValue.toStdString();
        vector<string> Attrs;
        Attrs.push_back(strDefault);
        m_pCon->addPrinterOptionDefault(m_strName.toStdString().c_str(),
                                        "finishings", &Attrs);
    }
    catch (const std::runtime_error& e)
    {
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
    }
    */

    setOptionValue("BindEdge", strValue);
}

QVector<QMap<QString, QString>> DPrinter::getBindEdgeChooses()
{
    return getOptionChooses("BindEdge");
}

QString DPrinter::getFinishings()
{
    QString strFinishings = "";

    try {
        vector<string> requestAttrs;
        requestAttrs.push_back("finishings-default");
        map<string, string> attrs = m_pCon->getPrinterAttributes(m_strName.toStdString().c_str(),
                                                                 nullptr, &requestAttrs);

        for (auto iter = attrs.begin(); iter != attrs.end(); iter++)
        {
            if(0 == iter->first.compare("finishings-default"))
            {
                strFinishings = QString::fromStdString(iter->second.data());

                if(strFinishings.isEmpty())
                {
                    strFinishings = " ";
                }
                else
                {
                    strFinishings = strFinishings.remove('i');
                    strFinishings =strFinishings.trimmed();

                    if(strFinishings == "3")
                    {
                        strFinishings = "none";
                    }
                    else if(strFinishings == "51")
                    {
                        strFinishings = "left";
                    }
                    else if(strFinishings == "50")
                    {
                        strFinishings = "right";
                    }
                }
            }
        }
    } catch (const std::runtime_error &e) {
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
        strFinishings.clear();
    }

    return strFinishings;
}

void DPrinter::setFinishings(const QString& strValue)
{
    try
    {
        string strDefault;

        if(strValue == "left")
        {
            strDefault = "51";
        }
        else if(strValue == "top")
        {
            strDefault = "50";
        }
        else
        {
            strDefault = "3";
        }

        vector<string> Attrs;
        Attrs.push_back(strDefault);
        m_pCon->addPrinterOptionDefault(m_strName.toStdString().c_str(),
                                        "finishings", &Attrs);
    }
    catch (const std::runtime_error& e)
    {
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
    }
}

QVector<QMap<QString, QString>> DPrinter::getFinishingsChooses()
{
    QVector<QMap<QString, QString>> vecChoose;

    try
    {
        vector<string> requestAttrs;
        requestAttrs.push_back("finishings-supported");
        map<string, string> attrs = m_pCon->getPrinterAttributes(m_strName.toStdString().c_str(),
                                                                 nullptr, &requestAttrs);

        for (auto iter = attrs.begin(); iter != attrs.end(); iter++)
        {
            QMap<QString, QString> choose;
            QString strChoose = QString::fromStdString(iter->second);
            QStringList strChooses = strChoose.split('\0');
            DPrinterManager* pManger = DPrinterManager::getInstance();

            for (int i = 0; i < strChooses.size(); i++)
            {
                QString strTemp = strChooses[i].remove('`');
                strTemp = strChooses[i].remove('i');
                strTemp = strTemp.trimmed();

                if(0 == strTemp.compare("3"))
                {
                    choose[QString::fromStdString("text")] = pManger->translateLocal("StapleLocation_Combo", "Bind (none)");
                    choose[QString::fromStdString("choice")] = "none";
                }
                else if(0 == strTemp.compare("50"))
                {
                    choose[QString::fromStdString("text")] = pManger->translateLocal("StapleLocation_Combo", "Bind (top)");
                    choose[QString::fromStdString("choice")] = "top";
                }
                else if(0 == strTemp.compare("51"))
                {
                    choose[QString::fromStdString("text")] = pManger->translateLocal("StapleLocation_Combo", "Bind (left)");
                    choose[QString::fromStdString("choice")] = "left";
                }
                else
                {
                    continue;
                }

                vecChoose.push_back(choose);
            }
        }
    }
    catch (const std::runtime_error& e)
    {
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
        vecChoose.clear();
    }

    return vecChoose;
}

QString DPrinter::getStapleLocation()
{
    return getOptionValue("StapleLocation");
}

void DPrinter::setStapleLoaction(const QString& strVal)
{
    setOptionValue("StapleLocation", strVal);
}

QVector<QMap<QString, QString>> DPrinter::getStapLocationChooses()
{
    return getOptionChooses("StapleLocation");
}

QString DPrinter::getResolution()
{
    return getOptionValue("Resolution");
}

void DPrinter::setResolution(const QString& strValue)
{
    setOptionValue("Resolution", strValue);
}

QVector<QMap<QString,QString>> DPrinter::getResolutionChooses()
{
    return getOptionChooses("Resolution");
}

QStringList DPrinter::getDefaultPpdOpts()
{
    QStringList strDefaultOptions;
    QString strValue = getPageSize();
    strDefaultOptions.push_back(strValue);
    strValue = getMediaType();
    strDefaultOptions.push_back(strValue);
    strValue = getColorModel();
    strDefaultOptions.push_back(strValue);
    strValue = getPrintQuality();
    strDefaultOptions.push_back(strValue);
    strValue = getInputSlot();
    strDefaultOptions.push_back(strValue);
    strValue = getDuplex();
    strDefaultOptions.push_back(strValue);
    return strDefaultOptions;
}

bool DPrinter::saveModify()
{
    bool bRet = false;

    try {
        g_cupsConnection->addPrinter(getName().toUtf8().data(), nullptr, nullptr, nullptr, nullptr, nullptr, &m_ppd);
    } catch (const std::runtime_error& e) {
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
        bRet = false;
    }

    m_bNeedSavePpd = false;
    return bRet;
}

bool DPrinter::needSavePpd()
{
    return m_bNeedSavePpd;
}

bool DPrinter::isConflict(const QString& strModule, const QString& strValue, QVector<CONFLICTNODE>& vecConflicts)
{
    bool bRet = false;
    std::vector<Constraint> vecCons = m_ppd.getConstraints();

    for (unsigned int i = 0; i < vecCons.size(); i++) {
        Constraint tempCons = vecCons[i];
        QString strOption1 = QString::fromStdString(tempCons.getOption1());

        if(strOption1 != strModule)
        {
            continue;
        }

        QString strChoice1 = QString::fromStdString(tempCons.getChoice1());

        if(strChoice1 != strValue)
        {
            continue;
        }

        CONFLICTNODE node;
        node.strOption = QString::fromStdString(tempCons.getOption2());
        node.strValue = QString::fromStdString(tempCons.getChoice2());
        vecConflicts.push_back(node);
        bRet = true;
    }

    return bRet;
}

 QVector<INSTALLABLEOPTNODE> DPrinter::getInstallableNodes()
 {
     QVector<INSTALLABLEOPTNODE> nodes;

     try {
         vector<Group> groups = m_ppd.getOptionGroups();

         for (unsigned int i = 0; i < groups.size(); i++) {
             Group grp = groups[i];
             QString strName = QString(grp.getName().data());

             if(strName == QString::fromStdString("InstallableOptions"))
             {
                 vector<Option>  opts = grp.getOptions();

                 for (unsigned int i = 0; i < opts.size(); i++) {
                     Option opt = opts[i];
                     INSTALLABLEOPTNODE node;
                     node.strOptName = QString(opt.getKeyword().data());
                     node.strOptText = QString(opt.getText().data());
                     node.strDefaultValue = getOptionValue(node.strOptName);
                     node.vecChooseableValues = getOptionChooses(node.strOptName);
                     nodes.push_back(node);
                 }
             }
         }
     } catch (const std::runtime_error &e) {
         qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
     }
     return nodes;
 }

 void DPrinter::setInstallableNodeValue(const QString& strOpt, const QString& strValue)
 {
     try {
         setOptionValue(strOpt, strValue);
     } catch (const std::runtime_error &e) {
         qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
     }
 }

 QVector<GENERALOPTNODE> DPrinter::getGeneralNodes()
 {
     QVector<GENERALOPTNODE> nodes;

     try {
         vector<Group> groups = m_ppd.getOptionGroups();

         for (unsigned int i = 0; i < groups.size(); i++) {
             Group grp = groups[i];
             QString strName = QString(grp.getName().data());

             if(strName == QString::fromStdString("General"))
             {
                 vector<Option>  opts = grp.getOptions();

                 for (unsigned int i = 0; i < opts.size(); i++) {
                     Option opt = opts[i];
                     GENERALOPTNODE node;
                     node.strOptName = QString::fromStdString(opt.getKeyword());

                     if(node.strOptName == QString("ColorModel")||(node.strOptName == QString("PageRegion")))
                     {
                         continue;
                     }

                     node.strOptText = QString::fromStdString(opt.getText());
                     node.strOptText = node.strOptText.trimmed();
                     node.strDefaultValue = getOptionValue(node.strOptName);
                     node.vecChooseableValues = getOptionChooses(node.strOptName);
                     nodes.push_back(node);
                 }
             }
         }
     } catch (const std::runtime_error &e) {
         qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
     }
     return nodes;
 }

 void DPrinter::setGeneralNodeValue(const QString& strOpt, const QString& strValue)
 {
     try {
         setOptionValue(strOpt, strValue);
     } catch (const std::runtime_error &e) {
         qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
     }
 }

QString DPrinter::getOptionValue(const QString &strOptName)
{
    QString strDefault;

    try {
        Option opt = m_ppd.findOption(strOptName.toStdString().c_str());
        strDefault = QString::fromStdString(opt.getDefchoice());
    } catch (const std::runtime_error &e) {
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
        strDefault.clear();
    }

    return strDefault;
}

void DPrinter::setOptionValue(const QString &strOptName, const QString &strValue)
{
#ifdef _DEBUG
    cout << strOptName.c_str() << endl;
    cout << strValue.c_str() << endl;
#endif

    try {
        int iConflicts =  m_ppd.markOption(strOptName.toStdString().c_str(), strValue.toStdString().c_str());

        if(iConflicts != 0)
        {
            qDebug() << "conflict numbers:" << iConflicts;
            m_bNeedSavePpd = false;
        }
        else
        {
            m_bNeedSavePpd = true;
        }
    } catch (const std::runtime_error &e) {
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
        m_bNeedSavePpd = false;
    }
}

QVector<QMap<QString, QString>> DPrinter::getOptionChooses(const QString &strOptName)
{
    QVector<QMap<QString, QString>>retMap;

    try {
        Option opt = m_ppd.findOption(strOptName.toStdString().c_str());
        vector<map<string, string>> vecChoices = opt.getChoices();

        for (unsigned int i = 0; i < vecChoices.size(); i++) {
            map<string, string> mapChoices = vecChoices[i];
            QMap<QString, QString> mapTrans;

            for (auto iter = mapChoices.begin(); iter != mapChoices.end(); iter++) {
                mapTrans.insert(QString::fromStdString(iter->first), QString::fromStdString(iter->second));
            }

            retMap.push_back(mapTrans);
        }
    } catch (const std::runtime_error &e) {
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
        retMap.clear();
    }

    return retMap;
}

QString DPrinter::getColorAttr()
{
    QString strDefault;

    try {
        Attribute attr = m_ppd.findAttr("DefaultColorSpace", nullptr);
        strDefault = QString::fromStdString(attr.getValue());
    } catch (const std::runtime_error &e) {
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
    }

    return strDefault;
}

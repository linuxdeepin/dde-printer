/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     shenfusheng_cm <shenfusheng_cm@deepin.com>
 *
 * Maintainer: shenfusheng_cm <shenfusheng_cm@deepin.com>
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

#include "dpropertysetdlg.h"
#include "dprinterpropertytemplate.h"

#include <qsettingbackend.h>
#include <dsettings.h>
#include <DSettingsOption>
#include <DSettingsOption>
#include <DSettingsWidgetFactory>
#include <DLabel>
#include <DListView>
#include <DPushButton>
#include <DDialog>
#include <DComboBox>
#include <DLineEdit>

#include <QDir>
#include <QTemporaryFile>
#include <QDebug>
#include <QFont>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLocale>
#include <QPalette>

#include "../util/dprintermanager.h"
#include "../util/dprinter.h"

#define FLAGITEMSPACE "\b\b\b\b\b\b\b\b"
#define ITEMWIDTH 100
#define ITEMHEIGHT 56
#define UNSUPPORTED "-1"

enum COMBOITEMROLE
{
    VALUEROLE = Qt::UserRole + 1,
    BACKCOLORROLE
};

DPropertySetDlg::DPropertySetDlg(QWidget *pParent) : DSettingsDialog(pParent)
{
    initUI();
    initConnection();
}

void DPropertySetDlg::initUI()
{
    widgetFactory()->registerWidget("custom-label", [](QObject * obj) -> QWidget* {
        if (DSettingsOption *option = qobject_cast<DSettingsOption *>(obj))
        {
            QString strVal = option->data("text").toString();
            qDebug() << "create custom label:" << option->value();
            DLabel *pLable = new DLabel(FLAGITEMSPACE + strVal);
            return pLable;
        }

        return nullptr;
    });

    widgetFactory()->registerWidget("custom-lineedit", [this](QObject * obj) -> QWidget* {
        if (DSettingsOption *option = qobject_cast<DSettingsOption *>(obj))
        {
            QString strName = option->name();
            QLineEdit* pLineEdit = new QLineEdit;
            pLineEdit->setObjectName(strName);
            m_mapOfListWidget[strName] = pLineEdit;
            return pLineEdit;
        }

        return nullptr;
    });

    widgetFactory()->registerWidget("custom-listview", [this](QObject * obj) -> QWidget* {
        if (DSettingsOption *option = qobject_cast<DSettingsOption *>(obj))
        {
            qDebug() << "create custom list:" << option->value();
            DListView *pListView = new DListView;
            //pListView->setMinimumSize(QSize(300,200));
            QStandardItemModel *pModel = new QStandardItemModel(pListView);
            QString strName = option->name();
            pListView->setObjectName(strName);

            auto itemdata = option->data(QString::fromStdString("items"));

            if(itemdata.type() == QVariant::StringList)
            {
                QStringList strItems = itemdata.toStringList();

                for(int i = 0 ; i< strItems.size(); i++)
                {
                    QString strItem = strItems[i];
                    QStandardItem *pItem = new QStandardItem(FLAGITEMSPACE + strItem);
                    pItem->setSizeHint(QSize(ITEMWIDTH, ITEMHEIGHT));
                    pModel->appendRow(pItem);
                }
            }

            pListView->setModel(pModel);
            m_mapOfListWidget[strName] = pListView;
            return pListView;
        }

        return nullptr;
    });

    widgetFactory()->registerWidget("custom-combobox", [this](QObject * obj) -> QWidget* {
        if (DSettingsOption *option = qobject_cast<DSettingsOption *>(obj))
        {
            qDebug() << "create custom combobox:" << option->value();
            DComboBox *pCombo = new DComboBox;
            QString strName = option->name();
            pCombo->setObjectName(strName);
            m_mapOfListWidget[strName] = pCombo;
            return pCombo;
        }

        return nullptr;
    });

    widgetFactory()->registerWidget("custom-buttons", [this](QObject * obj) -> QWidget* {
        if (DSettingsOption *option = qobject_cast<DSettingsOption *>(obj))
        {
            qDebug() << "create custom button:" << option->value();
            QWidget* pWidget = new QWidget;
            auto itemdata = option->data(QString::fromStdString("items"));

            if(itemdata.type() == QVariant::StringList)
            {
                QStringList strItems = itemdata.toStringList();
                QHBoxLayout* pHlayout = new QHBoxLayout;

                for(int i = 0; i < strItems.size(); i++)
                {
                    QString strItem = strItems[i];
                    DPushButton* pBtn = new DPushButton(strItem);
                    pBtn->setObjectName(strItem);
                    m_mapOfListWidget[strItem] = pBtn;
                    pHlayout->addWidget(pBtn);
                }

                pWidget->setLayout(pHlayout);
            }

            return pWidget;
        }

        return nullptr;
    });

    QTemporaryFile tmpFile;
    tmpFile.open();
    QString strTmpFileName = tmpFile.fileName();
    auto backend = new Dtk::Core::QSettingBackend(strTmpFileName);
    QPointer<DSettings> settings;
    QVector<QString> vecOption;
    vecOption.push_back(tr("Print Settings"));
    vecOption.push_back(tr("Driver"));
    vecOption.push_back(tr("URI"));
    vecOption.push_back(tr("Location"));
    vecOption.push_back(tr("Description"));
    vecOption.push_back(tr("ColorMode"));
    vecOption.push_back(tr("Resolution"));
    vecOption.push_back(tr("Output Quality"));
    vecOption.push_back(tr("Paper Source"));
    vecOption.push_back(tr("Paper Type"));
    vecOption.push_back(tr("Paper Size"));
    vecOption.push_back(tr("Duplex"));
    vecOption.push_back(tr("Margins"));
    vecOption.push_back(tr("Orientation"));
    vecOption.push_back(tr("Page Order"));
    vecOption.push_back(tr("Binding"));

    QString strJson = generatePropertyDialogJson(vecOption);
    settings = Dtk::Core::DSettings::fromJson(strJson.toUtf8());
    settings->setBackend(backend);
    updateSettings(settings);
    resize(682, 546);
    m_bShow = false;
    setResetVisible(false);
    setWindowIcon(QIcon(":/images/dde-printer.svg"));
}

void DPropertySetDlg::initConnection()
{
    //PageSize_Combo
    QComboBox *pCombo = qobject_cast<QComboBox *>(m_mapOfListWidget[QString::fromStdString("PageSize_Combo")]);
    //QObject::connect(pCombo, &QComboBox::currentIndexChanged, this, &DPropertySetDlg::c_pageSizeCombo_clicked);
    QObject::connect(pCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(pageSizeCombo_clicked(int)));

    //Direction_Combo
    pCombo = qobject_cast<DComboBox *>(m_mapOfListWidget[QString::fromStdString("Orientation_Combo")]);
    QObject::connect(pCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(paperOrientation_clicked(int)));

    //Duplex_Combo
    pCombo = qobject_cast<DComboBox *>(m_mapOfListWidget[QString::fromStdString("Duplex_Combo")]);
    QObject::connect(pCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(duplexPrintCombo_clicked(int)));

    //ColorMode_Combo
    pCombo = qobject_cast<DComboBox *>(m_mapOfListWidget[QString::fromStdString("ColorMode_Combo")]);
    QObject::connect(pCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(colorModeCombo_clicked(int)));

    //PrintOrder_Combo
    pCombo = qobject_cast<DComboBox *>(m_mapOfListWidget[QString::fromStdString("PrintOrder_Combo")]);
    QObject::connect(pCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(printOrderCombo_clicked(int)));

    //PaperOrigin_Combo
    pCombo = qobject_cast<DComboBox *>(m_mapOfListWidget[QString::fromStdString("PaperOrigin_Combo")]);
    QObject::connect(pCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(paperOriginCombo_clicked(int)));

    //PaperType_Combo
    pCombo = qobject_cast<DComboBox *>(m_mapOfListWidget[QString::fromStdString("PaperType_Combo")]);
    QObject::connect(pCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(paperTypeCombo_clicked(int)));

    //OutputQuanlity_Combo
    pCombo = qobject_cast<DComboBox *>(m_mapOfListWidget[QString::fromStdString("OutputQuanlity_Combo")]);
    QObject::connect(pCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(outputQuanlity_clicked(int)));

    //Finishings_Combo
    pCombo = qobject_cast<DComboBox *>(m_mapOfListWidget[QString::fromStdString("Finishings_Combo")]);
    QObject::connect(pCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(printFinishingsCombo_clicked(int)));

    //BindEdge_Combo
    pCombo = qobject_cast<DComboBox *>(m_mapOfListWidget[QString::fromStdString("BindEdge_Combo")]);
    QObject::connect(pCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(printBindEdgeCombo_clicked(int)));

    //Resolution_Combo
    pCombo = qobject_cast<DComboBox *>(m_mapOfListWidget[QString::fromStdString("Resolution_Combo")]);
    QObject::connect(pCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(printResolutionCombo_clicked(int)));

    //Uri_LineEdit
    QLineEdit* pLineEdit = qobject_cast<QLineEdit *>(m_mapOfListWidget[QString::fromStdString("URI_LineEdit")]);
    QObject::connect(pLineEdit, SIGNAL(editingFinished()), this, SLOT(printUriUI_EditFinished()));

    //Location_LineEdit
    pLineEdit = qobject_cast<QLineEdit *>(m_mapOfListWidget[QString::fromStdString("Location_LineEdit")]);
    QObject::connect(pLineEdit, SIGNAL(editingFinished()), this, SLOT(printLocationUI_EditFinished()));

    //Description_LineEdit
    pLineEdit = qobject_cast<QLineEdit *>(m_mapOfListWidget[QString::fromStdString("Description_LineEdit")]);
    QObject::connect(pLineEdit, SIGNAL(editingFinished()), this, SLOT(printDescriptionUI_EditFinished()));
}

void DPropertySetDlg::updateComboByName(const QString &strWidgetName, const QString &strDefault, const QVector<QMap<QString, QString>> &vecChoices)
{
    QComboBox *pCombo = qobject_cast<DComboBox*>(m_mapOfListWidget[strWidgetName]);

    if (nullptr == pCombo) {
        return;
    }

    if(vecChoices.size() > 0){
        int iIndex = -1;

        for (int i = 0; i < vecChoices.size(); i++) {
            QMap<QString, QString> mapValues = vecChoices[i];
            QString strText = mapValues[QString::fromStdString("text")];
            QString strChoice = mapValues[QString::fromStdString("choice")];
            DPrinterManager* pManger = DPrinterManager::getInstance();
            strText =  strText.trimmed();
            strText = pManger->translateLocal(strWidgetName, strText);

            if ((!strText.isEmpty()) && (!strChoice.isEmpty())) {
                if (strChoice == strDefault) {
                    iIndex = i;
                }

                pCombo->addItem(strText);
                pCombo->setItemData(i, strChoice, COMBOITEMROLE::VALUEROLE);
            }
        }

        pCombo->setCurrentIndex(iIndex);
    }
    else{
        DPrinterManager* pManger = DPrinterManager::getInstance();
        QString strText = pManger->translateLocal(strWidgetName, "None");
        pCombo->addItem(strText);
        pCombo->setItemData(0, UNSUPPORTED, COMBOITEMROLE::VALUEROLE);
    }
}

void DPropertySetDlg::updateDriverUI(const QString &strDrive)
{
    QLineEdit *pLineEdit = qobject_cast<QLineEdit*>(m_mapOfListWidget[QString::fromStdString("Driver_LineEdit")]);

    if (nullptr == pLineEdit) {
        return;
    }

    /*
    int iPos = strDrive.lastIndexOf(QDir::separator());
    QString strText = strDrive.right(strDrive.length() - iPos -1);
    iPos = strText.lastIndexOf(".");
    QString strLast = strText.right(strText.length() - iPos);

    if(strLast.contains(".ppd"))
    {
        strText = strText.left(iPos);
    }
    */

    pLineEdit->setText(strDrive);
    pLineEdit->setReadOnly(true);
}

void DPropertySetDlg::updateUriUI(const QString& strUri)
{
    QLineEdit *pLineEdit = qobject_cast<QLineEdit *>(m_mapOfListWidget[QString::fromStdString("URI_LineEdit")]);

    if (nullptr == pLineEdit) {
        return;
    }

    pLineEdit->setText(strUri);
    pLineEdit->setReadOnly(false);
}

void DPropertySetDlg::updateLocationUI(const QString& strLocation)
{
    QLineEdit *pLineEdit = qobject_cast<QLineEdit *>(m_mapOfListWidget[QString::fromStdString("Location_LineEdit")]);

    if (nullptr == pLineEdit) {
        return;
    }

    pLineEdit->setText(strLocation);
    pLineEdit->setReadOnly(false);
}

void DPropertySetDlg::updateDescriptionUI(const QString& strDescription)
{
    QLineEdit *pLineEdit = qobject_cast<QLineEdit *>(m_mapOfListWidget[QString::fromStdString("Description_LineEdit")]);

    if (nullptr == pLineEdit) {
        return;
    }

    pLineEdit->setText(strDescription);
    pLineEdit->setReadOnly(false);
}

void DPropertySetDlg::updatePageSizeCombo(const QString &strDefault, const QVector<QMap<QString, QString>> &vecChoices)
{
    m_mapInitUIValue[QString::fromStdString("PageSize_Combo")] = strDefault;
    m_mapDynamicUIValue[QString::fromStdString("PageSize_Combo")] = strDefault;
    updateComboByName(QString::fromStdString("PageSize_Combo"), strDefault, vecChoices);
}

void DPropertySetDlg::setPrinterName(const QString &strName)
{
    m_strPrinterName = strName;
}

void DPropertySetDlg::updateViews()
{
    DPrinterManager *pManager = DPrinterManager::getInstance();
    DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

    if (pDest != nullptr) {
        if (DESTTYPE::PRINTER == pDest->getType()) {
            DPrinter *pPrinter = static_cast<DPrinter *>(pDest);

            //Update Driver UI
            QString strDriver = pPrinter->getPrinterMakeAndModel();
            updateDriverUI(strDriver);

            //Update Uri UI
            QString strUri = pPrinter->getPrinterUri();
            updateUriUI(strUri);

            //Update Location UI
            QString strLoc = pPrinter->printerLocation();
            updateLocationUI(strLoc);

            //Update Description UI
            QString strDes =  pPrinter->printerInfo();
            updateDescriptionUI(strDes);

            //Update PageSize UI
            QVector<QMap<QString, QString>> vecChoice = pPrinter->getPageSizeChooses();
            QString strDefault = pPrinter->getPageSize();
            updatePageSizeCombo(strDefault, vecChoice);

            //Update Orientation UI
            strDefault = pPrinter->getPageOrientation();
            vecChoice = pPrinter->getPageOrientationChooses();
            updateOrientationCombo(strDefault, vecChoice);

            //Update Duplex Setting UI
            vecChoice = pPrinter->getDuplexChooses();
            strDefault = pPrinter->getDuplex();
            updateDuplexPrintCombo(strDefault, vecChoice);

            //Update ColorModel UI
            vecChoice = pPrinter->getColorModelChooses();
            strDefault = pPrinter->getColorModel();
            updateColorModeCombo(strDefault, vecChoice);

            //Update PrintOrder UI
            strDefault = pPrinter->getPageOutputOrder();
            vecChoice = pPrinter->getPageOutputOrderChooses();
            updatePrintOrderCombo(strDefault, vecChoice);

            //Update PaperOrigin UI
            vecChoice = pPrinter->getInputSlotChooses();
            strDefault = pPrinter->getInputSlot();
            updatePaperOriginCombo(strDefault, vecChoice);

            //Update PaperType UI
            vecChoice = pPrinter->getMediaTypeChooses();
            strDefault = pPrinter->getMediaType();
            updatePaperTypeCombo(strDefault, vecChoice);

            //Update OutputQuanlity UI
            vecChoice = pPrinter->getPrintQualityChooses();
            strDefault = pPrinter->getPrintQuality();
            updateOutputQuanlityCombo(strDefault, vecChoice);

            //Update Finishings_Combo UI
            vecChoice = pPrinter->getFinishingsChooses();
            strDefault = pPrinter->getFinishings();
            updateFinishingsCombo(strDefault, vecChoice);

            //Update BindEdge UI
            vecChoice = pPrinter->getBindEdgeChooses();
            strDefault = pPrinter->getBindEdgeOption();
            updateBindEdgeCombo(strDefault, vecChoice);

            //Update Resolution UI
            vecChoice = pPrinter->getResolutionChooses();
            strDefault = pPrinter->getResolution();
            updateResolutionCombo(strDefault, vecChoice);
            initConflictMap();
        }
    }
}

void DPropertySetDlg::updateOrientationCombo(const QString & strDefault, const QVector<QMap<QString, QString>> &vecChoices)
{
    updateComboByName(QString::fromStdString("Orientation_Combo"), strDefault, vecChoices);
}

void DPropertySetDlg::updateDuplexPrintCombo(const QString &strDefault, const QVector<QMap<QString, QString>> &vecChoices)
{
    m_mapInitUIValue[QString::fromStdString("Duplex_Combo")] = strDefault;
    m_mapDynamicUIValue[QString::fromStdString("Duplex_Combo")] = strDefault;
    updateComboByName(QString::fromStdString("Duplex_Combo"), strDefault, vecChoices);
}

void DPropertySetDlg::updateColorModeCombo(const QString& strDefault, const QVector<QMap<QString, QString>>& vecChoices)
{
    m_mapInitUIValue[QString::fromStdString("ColorMode_Combo")] = strDefault;
    m_mapDynamicUIValue[QString::fromStdString("ColorMode_Combo")] = strDefault;
    updateComboByName(QString::fromStdString("ColorMode_Combo"), strDefault, vecChoices);
}

void DPropertySetDlg::updatePrintOrderCombo(const QString &strDefault, const QVector<QMap<QString, QString>> &vecChoices)
{
    updateComboByName(QString::fromStdString("PrintOrder_Combo"), strDefault, vecChoices);
}

void DPropertySetDlg::updatePaperOriginCombo(const QString &strDefault, const QVector<QMap<QString, QString>> &vecChoices)
{
    m_mapInitUIValue[QString::fromStdString("PaperOrigin_Combo")] = strDefault;
    m_mapDynamicUIValue[QString::fromStdString("PaperOrigin_Combo")] = strDefault;
    updateComboByName(QString::fromStdString("PaperOrigin_Combo"), strDefault, vecChoices);
}

void DPropertySetDlg::updatePaperTypeCombo(const QString &strDefault, const QVector<QMap<QString, QString>> &vecChoices)
{
    m_mapInitUIValue[QString::fromStdString("PaperType_Combo")] = strDefault;
    m_mapDynamicUIValue[QString::fromStdString("PaperType_Combo")] = strDefault;
    updateComboByName(QString::fromStdString("PaperType_Combo"), strDefault, vecChoices);
}

void DPropertySetDlg::updateOutputQuanlityCombo(const QString &strDefault, const QVector<QMap<QString, QString>> &vecChoices)
{
    m_mapInitUIValue[QString::fromStdString("OutputQuanlity_Combo")] = strDefault;
    m_mapDynamicUIValue[QString::fromStdString("OutputQuanlity_Combo")] = strDefault;
    updateComboByName(QString::fromStdString("OutputQuanlity_Combo"), strDefault, vecChoices);
}

void DPropertySetDlg::updateFinishingsCombo(const QString& strDefault, const QVector<QMap<QString, QString>>& vecChoices)
{
    updateComboByName(QString::fromStdString("Finishings_Combo"), strDefault, vecChoices);
}

void DPropertySetDlg::updateBindEdgeCombo(const QString& strDefault, const QVector<QMap<QString, QString>>& vecChoices)
{
    m_mapInitUIValue[QString::fromStdString("BindEdge_Combo")] = strDefault;
    m_mapDynamicUIValue[QString::fromStdString("BindEdge_Combo")] = strDefault;
    updateComboByName(QString::fromStdString("BindEdge_Combo"), strDefault, vecChoices);
}

void DPropertySetDlg::updateResolutionCombo(const QString& strDefault, const QVector<QMap<QString, QString>>& vecChoices)
{
    m_mapInitUIValue[QString::fromStdString("Resolution_Combo")] = strDefault;
    m_mapDynamicUIValue[QString::fromStdString("Resolution_Combo")] = strDefault;
    updateComboByName(QString::fromStdString("Resolution_Combo"), strDefault, vecChoices);
}

QString DPropertySetDlg::changeComboSelectionByName(const QString &strComboName, int iIndex)
{
    DComboBox *pCombo = qobject_cast<DComboBox*>(m_mapOfListWidget[strComboName]);

    if (nullptr == pCombo) {
        return UNSUPPORTED;
    }

    QString strChoice = pCombo->itemData(iIndex, COMBOITEMROLE::VALUEROLE).toString();
    QString strText = pCombo->currentText();

    if(strChoice == UNSUPPORTED)
    {
        return UNSUPPORTED;
    }

    //依据模块进行冲突判断
    vector<CONFLICTPAIR> vecConflictOptionPairs;
    checkAllConflicts(m_setConflictOptions, vecConflictOptionPairs);


    if(m_setConflictOptions.size() > 0)
    {
        showConflictDlg(vecConflictOptionPairs);
        strChoice = UNSUPPORTED;
    }
    else {
        m_mapDynamicUIValue[strComboName] = strChoice;
    }

    updateComboByConflits(m_setConflictOptions);
    return strChoice;
}

bool DPropertySetDlg::haveConflict(const QString& strModule, const QString& strValue,
                                 QVector<CONFLICTNODE>& vecConflictNode)
{
    bool bRet = true;
    DPrinterManager *pManager = DPrinterManager::getInstance();
    DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

    if (DESTTYPE::PRINTER == pDest->getType())
    {
        DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
        bRet = pPrinter->isConflict(strModule, strValue, vecConflictNode);
    }

    return bRet;
}

bool DPropertySetDlg::isConflict(const QString& strConflictOption, const QString& strConflictValue)
{
    QString strWidgetName = m_mapOfConflict.value(strConflictOption);

    if(strWidgetName.isEmpty())
    {
        return false;
    }

    QString strCurrentValue;
    QComboBox* pCombo = qobject_cast<QComboBox*>(m_mapOfListWidget[strWidgetName]);
    int iIndex = pCombo->currentIndex();
    strCurrentValue = pCombo->itemData(iIndex, COMBOITEMROLE::VALUEROLE).toString();
    return (strCurrentValue == strConflictValue);
}

void DPropertySetDlg::initConflictMap()
{
    m_mapOfConflict.clear();
    m_mapOfConflict.insert(QString::fromStdString("Duplex"), QString::fromStdString("Duplex_Combo"));
    m_mapOfConflict.insert(QString::fromStdString("MediaType"), QString::fromStdString("PaperType_Combo"));
    m_mapOfConflict.insert(QString::fromStdString("PageSize"), QString::fromStdString("PageSize_Combo"));
    m_mapOfConflict.insert(QString::fromStdString("InputSlot"), QString::fromStdString("PaperOrigin_Combo"));
    m_mapOfConflict.insert(QString::fromStdString("BindEdge"), QString::fromStdString("BindEdge_Combo"));
}

void DPropertySetDlg::showConflictDlg(const vector<CONFLICTPAIR>& vecConflictPairs)
{
    QWidget* pWidget = new QWidget;
    DLabel* pLabel1 = new DLabel(tr("Options conflict!"));
    DFontSizeManager::instance()->bind(pLabel1, DFontSizeManager::T6);
    QFont font = pLabel1->font();
    font.setFamily("SourceHanSansSC-Bold");
    pLabel1->setFont(font);
    pLabel1->setAlignment(Qt::AlignCenter);
    DLabel* pLabel2 = new DLabel(tr("Please resolve the conflict first, and then save the changes."));
    pLabel2->setFont(font);
    pLabel2->setAlignment(Qt::AlignCenter);
    DPrinterManager* pManager = DPrinterManager::getInstance();
    QString strOpt1 = pManager->translateLocal(m_mapOfConflict.value(vecConflictPairs[0].strOpt1), vecConflictPairs[0].strOpt1);
    QString strOpt2 = pManager->translateLocal(m_mapOfConflict.value(vecConflictPairs[0].strOpt2), vecConflictPairs[0].strOpt2);
    QLabel* pLabel3 = new DLabel(tr("Conflict:") + strOpt1 + "\t,\t" + strOpt2);
    pLabel3->setFont(font);
    pLabel3->setAlignment(Qt::AlignCenter);
    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->addWidget(pLabel1);
    pLayout->addWidget(pLabel2);
    pLayout->addWidget(pLabel3);
    pWidget->setLayout(pLayout);
    DDialog dialog(this);
    //dialog.setFixedSize(450, 172);
    dialog.addContent(pWidget);
    dialog.addButton(tr("Confirm"));
    dialog.setIcon(QPixmap(":/images/warning_logo.svg"));
    dialog.exec();
}

void DPropertySetDlg::updateComboByConflits(const QSet<QString>& nodes)
{
    for (auto iter = m_mapOfConflict.begin(); iter != m_mapOfConflict.end(); iter++) {

        if(nodes.contains(iter.key()))
        {
            continue;
        }

        QString strWidgetName = iter.value();
        QComboBox* pCombo = qobject_cast<QComboBox*>(m_mapOfListWidget.value(strWidgetName));
        QPalette pal = pCombo->palette();
        pal.setColor(QPalette::ButtonText, Qt::black);
        pCombo->setPalette(pal);
    }

    foreach(QString strOpt, nodes)
    {
        QString strWidgetName = m_mapOfConflict.value(strOpt);
        QComboBox* pCombo = qobject_cast<QComboBox*>(m_mapOfListWidget.value(strWidgetName));
        QPalette pal = pCombo->palette();
        pal.setColor(QPalette::ButtonText, Qt::red);
        pCombo->setPalette(pal);
    }
}

void DPropertySetDlg::checkAllConflicts(QSet<QString>& setOptions, vector<CONFLICTPAIR>& vecOptionPairs)
{
    bool bHaveConflict = false;
    CONFLICTNODE node;
    m_setConflictOptions.clear();

    for(auto iter = m_mapOfConflict.begin(); iter != m_mapOfConflict.end(); iter++)
    {
        QComboBox* pCombo = qobject_cast<QComboBox*>(m_mapOfListWidget.value(iter.value()));

        if(pCombo == nullptr)
        {
            continue;
        }

        int iCurIndex = pCombo->currentIndex();
        QString strCheckValue = pCombo->itemData(iCurIndex, COMBOITEMROLE::VALUEROLE).toString();
        QString strCheckOption = iter.key();
        QVector<CONFLICTNODE> vecConflicts;
        bHaveConflict = haveConflict(strCheckOption, strCheckValue, vecConflicts);

        if(bHaveConflict)
        {
            bHaveConflict = false;

            for(int i = 0; i < vecConflicts.size(); i++)
            {
                node = vecConflicts[i];
                bHaveConflict = isConflict(node.strOption, node.strValue);

                if(bHaveConflict)
                {
                    setOptions.insert(strCheckOption);
                    setOptions.insert(node.strOption);
                    CONFLICTPAIR pair;
                    pair.strOpt1 = strCheckOption;
                    pair.strOpt2 = node.strOption;
                    vecOptionPairs.push_back(pair);
                }
            }
        }
    }
}

void DPropertySetDlg::pageSizeCombo_clicked(int iIndex)
{
    if (!m_bShow) {
        return;
    }

    QString strChoice = changeComboSelectionByName(QString::fromStdString("PageSize_Combo"), iIndex);

    if(strChoice == UNSUPPORTED){
        return;
    }

    if (!strChoice.isEmpty()) {
        DPrinterManager *pManager = DPrinterManager::getInstance();
        DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

        if (pDest != nullptr) {
            if (DESTTYPE::PRINTER == pDest->getType()) {
                DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
                pPrinter->setPageSize(strChoice);
            }
        }
    }
}

void DPropertySetDlg::paperOrientation_clicked(int index)
{
    if (!m_bShow) {
        return;
    }

    QString strChoice = changeComboSelectionByName(QString::fromStdString("Orientation_Combo"), index);

    if(strChoice == UNSUPPORTED)
    {
        return;
    }

    if (!strChoice.isEmpty()) {
        DPrinterManager *pManager = DPrinterManager::getInstance();
        DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

        if (pDest != nullptr) {
            if (DESTTYPE::PRINTER == pDest->getType()) {
                DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
                //need set direction
                strChoice = strChoice.trimmed();
                pPrinter->setPageOrientationChoose(strChoice);
            }
        }
    }
}

void DPropertySetDlg::duplexPrintCombo_clicked(int index)
{
    if (!m_bShow) {
        return;
    }

    QString strChoice = changeComboSelectionByName(QString::fromStdString("Duplex_Combo"), index);

    if(strChoice == UNSUPPORTED)
    {
        return;
    }

    if (!strChoice.isEmpty()) {
        DPrinterManager *pManager = DPrinterManager::getInstance();
        DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

        if (pDest != nullptr) {
            if (DESTTYPE::PRINTER == pDest->getType()) {
                DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
                pPrinter->setDuplex(strChoice);
            }
        }
    }
}

void DPropertySetDlg::colorModeCombo_clicked(int index)
{
    if (!m_bShow) {
        return;
    }

    QString strChoice = changeComboSelectionByName(QString::fromStdString("ColorMode_Combo"), index);

    if(strChoice == UNSUPPORTED)
    {
        return;
    }

    if (!strChoice.isEmpty()) {
        DPrinterManager *pManager = DPrinterManager::getInstance();
        DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

        if (pDest != nullptr) {
            if (DESTTYPE::PRINTER == pDest->getType()) {
                DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
                pPrinter->setColorModel(strChoice);
            }
        }
    }
}

void DPropertySetDlg::printOrderCombo_clicked(int index)
{
    if (!m_bShow) {
        return;
    }

    QString strChoice = changeComboSelectionByName(QString::fromStdString("PrintOrder_Combo"), index);

    if(strChoice == UNSUPPORTED)
    {
        return;
    }

    if (!strChoice.isEmpty()) {
        DPrinterManager *pManager = DPrinterManager::getInstance();
        DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

        if (pDest != nullptr) {
            if (DESTTYPE::PRINTER == pDest->getType()) {
                DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
                //need set Print Order;
                pPrinter->setPageOutputOrder(strChoice);
            }
        }
    }
}

void DPropertySetDlg::paperOriginCombo_clicked(int index)
{
    if (!m_bShow) {
        return;
    }

    QString strChoice = changeComboSelectionByName(QString::fromStdString("PaperOrigin_Combo"), index);

    if(strChoice == UNSUPPORTED)
    {
        return;
    }

    if (!strChoice.isEmpty()) {
        DPrinterManager *pManager = DPrinterManager::getInstance();
        DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

        if (pDest != nullptr) {
            if (DESTTYPE::PRINTER == pDest->getType()) {
                DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
                pPrinter->setInputSlot(strChoice);
            }
        }
    }
}

void DPropertySetDlg::paperTypeCombo_clicked(int index)
{
    if (!m_bShow) {
        return;
    }

    QString strChoice = changeComboSelectionByName(QString::fromStdString("PaperType_Combo"), index);

    if(strChoice == UNSUPPORTED)
    {
        return;
    }

    if (!strChoice.isEmpty()) {
        DPrinterManager *pManager = DPrinterManager::getInstance();
        DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

        if (pDest != nullptr) {
            if (DESTTYPE::PRINTER == pDest->getType()) {
                DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
                pPrinter->setMediaType(strChoice);
            }
        }
    }
}

void DPropertySetDlg::printFinishingsCombo_clicked(int index)
{
    if (!m_bShow) {
        return;
    }

    QString strChoice = changeComboSelectionByName(QString::fromStdString("Finishings_Combo"), index);

    if(strChoice == UNSUPPORTED)
    {
        return;
    }

    if (!strChoice.isEmpty()) {
        DPrinterManager *pManager = DPrinterManager::getInstance();
        DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

        if (pDest != nullptr) {
            if (DESTTYPE::PRINTER == pDest->getType()) {
                DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
                pPrinter->setBindEdgetOption(strChoice);
            }
        }
    }
}

void DPropertySetDlg::printBindEdgeCombo_clicked(int index)
{
    if (!m_bShow) {
        return;
    }

    QString strChoice = changeComboSelectionByName(QString::fromStdString("BindEdge_Combo"), index);

    if(strChoice == UNSUPPORTED)
    {
        return;
    }

    if (!strChoice.isEmpty()) {
        DPrinterManager *pManager = DPrinterManager::getInstance();
        DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

        if (pDest != nullptr) {
            if (DESTTYPE::PRINTER == pDest->getType()) {
                DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
                pPrinter->setBindEdgetOption(strChoice);
            }
        }
    }
}

void DPropertySetDlg::printResolutionCombo_clicked(int index)
{
    if (!m_bShow) {
        return;
    }

    QString strChoice = changeComboSelectionByName(QString::fromStdString("Resolution_Combo"), index);

    if(strChoice == UNSUPPORTED)
    {
        return;
    }

    if (!strChoice.isEmpty()) {
        DPrinterManager *pManager = DPrinterManager::getInstance();
        DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

        if (pDest != nullptr) {
            if (DESTTYPE::PRINTER == pDest->getType()) {
                DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
                pPrinter->setResolution(strChoice);
            }
        }
    }
}

void DPropertySetDlg::outputQuanlity_clicked(int index)
{
    if (!m_bShow) {
        return;
    }

    QString strChoice = changeComboSelectionByName(QString::fromStdString("OutputQuanlity_Combo"), index);

    if(strChoice == UNSUPPORTED)
    {
        return;
    }

    if (!strChoice.isEmpty()) {
        DPrinterManager *pManager = DPrinterManager::getInstance();
        DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

        if (pDest != nullptr) {
            if (DESTTYPE::PRINTER == pDest->getType()) {
                DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
                pPrinter->setPrintQuality(strChoice);
            }
        }
    }
}

void DPropertySetDlg::printUriUI_EditFinished()
{
    if(!m_bShow){
        return;
    }

    QLineEdit *pLineEdit = qobject_cast<QLineEdit*>(m_mapOfListWidget[QString::fromStdString("URI_LineEdit")]);

    if (nullptr == pLineEdit) {
        return;
    }

    QString strVal = pLineEdit->text();

    if(strVal.isEmpty())
    {
        return;
    }

    QRegExp reg("(\\S+)(://)(\\S+)");
    QRegExpValidator v(reg);
    int pos = 0;
    QValidator::State state = v.validate(strVal, pos);

    if (state != QValidator::Acceptable)
    {
        DDialog dialog;
        dialog.setFixedSize(QSize(400, 150));
        dialog.setMessage(tr("Invalid URI"));
        dialog.addButton(tr("Confirm"));
        dialog.setIcon(QPixmap(":/images/warning_logo.svg"));
        dialog.exec();
        return;
    }

    DPrinterManager *pManager = DPrinterManager::getInstance();
    DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

    if (pDest != nullptr) {
        if (DESTTYPE::PRINTER == pDest->getType()) {
            DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
            pPrinter->setPrinterUri(strVal);
        }
    }
}

void DPropertySetDlg::printLocationUI_EditFinished()
{
    if(!m_bShow){
        return;
    }

    QLineEdit *pLineEdit = qobject_cast<QLineEdit*>(m_mapOfListWidget[QString::fromStdString("Location_LineEdit")]);

    if (nullptr == pLineEdit) {
        return;
    }

    QString strVal = pLineEdit->text();

    if(strVal.isEmpty())
    {
        return;
    }

    DPrinterManager *pManager = DPrinterManager::getInstance();
    DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

    if (pDest != nullptr) {
        if (DESTTYPE::PRINTER == pDest->getType()) {
            DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
            pPrinter->setPrinterLocation(strVal);
        }
    }
}

void DPropertySetDlg::printDescriptionUI_EditFinished()
{
    if(!m_bShow){
        return;
    }

    QLineEdit *pLineEdit = qobject_cast<QLineEdit*>(m_mapOfListWidget[QString::fromStdString("Description_LineEdit")]);

    if (nullptr == pLineEdit) {
        return;
    }

    QString strVal = pLineEdit->text();

    if(strVal.isEmpty())
    {
        return;
    }

    DPrinterManager *pManager = DPrinterManager::getInstance();
    DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

    if (pDest != nullptr) {
        if (DESTTYPE::PRINTER == pDest->getType()) {
            DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
            pPrinter->setPrinterInfo(strVal);
        }
    }
}

void DPropertySetDlg::confirmBtn_clicked()
{

}

void DPropertySetDlg::cancelBtn_clicked()
{

}

void DPropertySetDlg::moveToParentCenter()
{
    QWidget *pParent = parentWidget();

    if (pParent == nullptr) {
        return;
    } else {
        QRect parentRc = pParent->geometry();
        QPoint parentCen = parentRc.center();
        QRect selfRc = geometry();
        QPoint selfCen = selfRc.center();
        QPoint selfCenInParent = mapToParent(selfCen);
        QPoint mvPhasor = parentCen - selfCenInParent;
        move(mvPhasor.x(), mvPhasor.y());
    }
}

bool DPropertySetDlg::isDriveBroken()
{
    bool bRet = true;
    DPrinterManager *pManager = DPrinterManager::getInstance();
    DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

    if (pDest != nullptr)
    {
        if (DESTTYPE::PRINTER == pDest->getType())
        {
            DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
            bRet = pPrinter->initPrinterPPD();
            bRet = !bRet;
        }
        else
        {
            bRet = false;
        }
    }

    return bRet;
}

void DPropertySetDlg::showEvent(QShowEvent *event)
{
    vector<CONFLICTPAIR> vecConflictOptionPairs;
    checkAllConflicts(m_setConflictOptions, vecConflictOptionPairs);


    if(m_setConflictOptions.size() > 0)
    {
        showConflictDlg(vecConflictOptionPairs);
        updateComboByConflits(m_setConflictOptions);
    }

    m_bShow = true;
    DSettingsDialog::showEvent(event);
}

void DPropertySetDlg::closeEvent(QCloseEvent *event)
{
    DPrinterManager *pManager = DPrinterManager::getInstance();
    DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);
    DPrinter *pPrinter = nullptr;

    if (pDest != nullptr)
    {
        if (DESTTYPE::PRINTER == pDest->getType())
        {
            pPrinter = static_cast<DPrinter *>(pDest);

            if(!pPrinter->needSavePpd())
            {
                return;
            }
            else
            {
                bool bSame = true;

                for(auto iter = m_mapInitUIValue.begin(); iter != m_mapInitUIValue.end(); iter++)
                {
                    QString strValue1 = iter.value();
                    QString strValue2 = m_mapDynamicUIValue[iter.key()];

                    if(strValue1 != strValue2)
                    {
                        bSame = false;
                        break;
                    }
                }
\
                if((!bSame) && (0 == m_setConflictOptions.size()))
                {
                    pPrinter->saveModify();
                }
            }
        }
    }

    DSettingsDialog::closeEvent(event);
}

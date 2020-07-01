/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     shenfusheng <shenfusheng_cm@deepin.com>
 *
 * Maintainer: shenfusheng <shenfusheng_cm@deepin.com>
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
#include "dprintersupplyshowdlg.h"
#include "dprintermanager.h"
#include "dprinter.h"

#include <DLabel>
#include <DProgressBar>
#include <DFrame>
#include <DWidget>
#include <DProgressBar>
#include <DFontSizeManager>
#include <DPushButton>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPalette>
#include <QTime>

DPrinterSupplyShowDlg::DPrinterSupplyShowDlg(const QString& strPrinterName, QWidget *parent):DDialog(parent)
  ,m_strPrinterName(strPrinterName)
{
    initColorTrans();
}

DPrinterSupplyShowDlg::~DPrinterSupplyShowDlg()
{

}

bool DPrinterSupplyShowDlg::isDriveBroken()
{
    bool bRet = true;
    DPrinterManager *pManager = DPrinterManager::getInstance();
    DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

    if (pDest != nullptr) {
        if (DESTTYPE::PRINTER == pDest->getType()) {
            DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
            bRet = pPrinter->isPpdFileBroken();
        } else {
            bRet = false;
        }
    }

    return bRet;
}

void DPrinterSupplyShowDlg::initUI()
{
    setIcon(QIcon(":/images/dde-printer.svg"));
    QVBoxLayout* pVlayout = new QVBoxLayout;
    pVlayout->setSpacing(0);
    DWidget* pWidget = new DWidget(this);
    QLabel* pLabel = new QLabel;
    DFontSizeManager::instance()->bind(pLabel, DFontSizeManager::T6, int(QFont::Medium));
    pLabel->setText(tr("Ink/Toner Status"));
    pVlayout->addWidget(pLabel, 0, Qt::AlignHCenter|Qt::AlignTop);

    if(isDriveBroken()){
        pVlayout->addStretch(500);
        QLabel* pLabel2 = new QLabel;
        pLabel2->setText(tr("Unknown amount"));
        DFontSizeManager::instance()->bind(pLabel2, DFontSizeManager::T4, int(QFont::Medium));
        QPalette pal = pLabel2->palette();
        QColor color = pal.color(QPalette::WindowText);
        pal.setColor(QPalette::WindowText, QColor(color.red(), color.green(), color.blue(), 160));
        pLabel2->setPalette(pal);
        QLabel* pLabel3 = new QLabel;
        pLabel3->setText(tr("Unable to get the remaining amount"));
        pVlayout->addWidget(pLabel2, 0, Qt::AlignHCenter);
        pVlayout->addSpacing(10);
        pVlayout->addWidget(pLabel3, 0, Qt::AlignHCenter);
        DFontSizeManager::instance()->bind(pLabel3, DFontSizeManager::T6, int(QFont::Light));
        pal = pLabel3->palette();
        color = pal.color(QPalette::WindowText);
        pal.setColor(QPalette::WindowText, QColor(color.red(), color.green(), color.blue(), 120));
        pLabel3->setPalette(pal);
    }
    else {
        if(canGetSupplyMsg()){
            pVlayout->addSpacing(20);
            bool bColor = isColorPrinter();

            for(int i = 0; i < m_supplyInfos.size(); i++){
                QWidget* pColorWidget = initColorSupplyItem(m_supplyInfos[i], bColor);
                pVlayout->addWidget(pColorWidget);
                pVlayout->addSpacing(1);
            }

            pVlayout->addSpacing(10);
            DLabel* pTimelLabel = new DLabel;
            DFontSizeManager::instance()->bind(pTimelLabel, DFontSizeManager::T8, int(QFont::ExtraLight));
            QTime time = QTime::currentTime();
            pTimelLabel->setText(tr("The amounts are estimated, last updated at %1:%2").arg(time.hour()).arg(time.minute()));
            pVlayout->addWidget(pTimelLabel, 0, Qt::AlignHCenter);
        }
    }

    pVlayout->addStretch(500);
    m_pConfirmBtn = new DPushButton();
    m_pConfirmBtn->setFixedSize(230, 36);
    DFontSizeManager::instance()->bind(m_pConfirmBtn, DFontSizeManager::T6, int(QFont::ExtraLight));
    m_pConfirmBtn->setText(tr("OK"));
    pVlayout->addWidget(m_pConfirmBtn, 0, Qt::AlignCenter|Qt::AlignBottom);
    pVlayout->setContentsMargins(10, 0, 10, 0);
    pWidget->setLayout(pVlayout);
    addContent(pWidget);
    setFixedSize(380, 356);
    moveToParentCenter();
}

void DPrinterSupplyShowDlg::initConnection()
{
    connect(m_pConfirmBtn, &DPushButton::clicked, this, &QDialog::accept);
}

void DPrinterSupplyShowDlg::moveToParentCenter()
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

QWidget* DPrinterSupplyShowDlg::initColorSupplyItem(const SUPPLYSDATA& info, bool bColor)
{
    QWidget* pWidget = new QWidget;
    pWidget->setAutoFillBackground(true);
    QPalette pal;
    pal.setColor(QPalette::Background, QColor(0, 0, 0, int(0.03 * 255)));
    pWidget->setPalette(pal);
    QHBoxLayout* pHlayout = new QHBoxLayout;
    pHlayout->setSpacing(10);

    if((info.colorant != 0)&&bColor){
        QPixmap colorPix(12, 12);
        colorPix.fill(Qt::transparent);
        QPainter painter(&colorPix);
        painter.setRenderHints(QPainter::Antialiasing|QPainter::SmoothPixmapTransform, true);
        painter.setBrush(QBrush(QColor(info.color)));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(QRect(0, 0, 12, 12), 4, 4);
        DLabel* pColorLabel = new DLabel;
        pColorLabel->setPixmap(colorPix);
        pHlayout->addWidget(pColorLabel);

        if((info.level < 0) || (info.level > 100)){
            pColorLabel->hide();
        }
    }

    QString strColorName = getTranslatedColor(info.colorName);
    DLabel* pLabel = new DLabel(strColorName, this);
    DFontSizeManager::instance()->bind(pLabel, DFontSizeManager::T6, int(QFont::ExtraLight));
    pHlayout->addWidget(pLabel);

    if(info.level <= 100)
    {
        QLabel* pImageLabel = new DLabel(this);
        QIcon icon = QIcon::fromTheme("dp_warning");
        QPixmap pix = icon.pixmap(QSize(14, 14));
        pImageLabel->setPixmap(pix);
        DProgressBar* pProcessBar = new DProgressBar;
        pProcessBar->setTextVisible(false);
        pProcessBar->setFixedSize(230, 8);
        pProcessBar->setRange(0, 100);
        pProcessBar->setValue(abs(info.level));
        pal.setColor(QPalette::Background, QColor(0, 0, 0, int(0.1 * 255)));
        pProcessBar->setAutoFillBackground(true);
        pProcessBar->setPalette(pal);
        pHlayout->addStretch(46);
        pHlayout->addWidget(pImageLabel, Qt::AlignRight);
        pHlayout->addWidget(pProcessBar, Qt::AlignRight);

        if(info.level > 0){
            if(info.level < 20){
                pImageLabel->show();
            }
            else {
                pImageLabel->hide();
            }
        }
        else {
            pImageLabel->hide();
        }
    }
    else
    {
        QIcon icon = QIcon::fromTheme("dp_caution");
        QPixmap pix = icon.pixmap(QSize(14, 14));
        DLabel* pImageLabel = new DLabel(this);
        pImageLabel->setPixmap(pix);
        DLabel* pTextLabel = new DLabel(tr("Unknown"),this);
        DFontSizeManager::instance()->bind(pTextLabel, DFontSizeManager::T8, int(QFont::ExtraLight));
        pHlayout->addSpacing(13);
        pHlayout->addWidget(pImageLabel, Qt::AlignRight);
        pHlayout->addWidget(pTextLabel, Qt::AlignRight);
        pHlayout->addSpacing(300);
    }

    pWidget->setLayout(pHlayout);
    pWidget->setFixedHeight(32);
    return pWidget;
}

bool DPrinterSupplyShowDlg::isColorPrinter()
{
    bool bRet = true;
    DPrinterManager *pManager = DPrinterManager::getInstance();
    DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

    if (pDest != nullptr) {
        if (DESTTYPE::PRINTER == pDest->getType()) {
            DPrinter *pPrinter = static_cast<DPrinter *>(pDest);
            bRet = pPrinter->canSetColorModel();
        } else {
            bRet = false;
        }
    }

    return bRet;
}

void DPrinterSupplyShowDlg::initColorTrans()
{
    m_mapColorTrans.clear();
    m_mapColorTrans.insert("Black", QObject::tr("Black"));
    m_mapColorTrans.insert("Blue", QObject::tr("Blue"));
    m_mapColorTrans.insert("Brown", QObject::tr("Brown"));
    m_mapColorTrans.insert("Cyan", QObject::tr("Cyan"));
    m_mapColorTrans.insert("Dark-gray", QObject::tr("Dark-gray"));
    m_mapColorTrans.insert("Dark gray", QObject::tr("Dark gray"));
    m_mapColorTrans.insert("Dark-yellow", QObject::tr("Dark-yellow"));
    m_mapColorTrans.insert("Dark yellow", QObject::tr("Dark yellow"));
    m_mapColorTrans.insert("Gold", QObject::tr("Gold"));
    m_mapColorTrans.insert("Gray", QObject::tr("Gray"));
    m_mapColorTrans.insert("Green", QObject::tr("Green"));
    m_mapColorTrans.insert("Light-black", QObject::tr("Light-black"));
    m_mapColorTrans.insert("Light black", QObject::tr("Light black"));
    m_mapColorTrans.insert("Light-cyan", QObject::tr("Light-cyan"));
    m_mapColorTrans.insert("Light cyan", QObject::tr("Light cyan"));
    m_mapColorTrans.insert("Light-gray", QObject::tr("Light-gray"));
    m_mapColorTrans.insert("Light gray", QObject::tr("Light gray"));
    m_mapColorTrans.insert("Light-magenta", QObject::tr("Light-magenta"));
    m_mapColorTrans.insert("Light magenta", QObject::tr("Light magenta"));
    m_mapColorTrans.insert("Magenta", QObject::tr("Magenta"));
    m_mapColorTrans.insert("Orange", QObject::tr("Orange"));
    m_mapColorTrans.insert("Red", QObject::tr("Red"));
    m_mapColorTrans.insert("Silver", QObject::tr("Silver"));
    m_mapColorTrans.insert("White", QObject::tr("White"));
    m_mapColorTrans.insert("Yellow", QObject::tr("Yellow"));
    m_mapColorTrans.insert("Waste", QObject::tr("Waste"));
}

QString DPrinterSupplyShowDlg::getTranslatedColor(const QString& strColor)
{
    QString strRet;

    if(m_mapColorTrans.contains(strColor)){
        strRet = m_mapColorTrans.value(strColor);
    }
    else {
        strRet = strColor;
    }

    return strRet;
}

bool DPrinterSupplyShowDlg::canGetSupplyMsg()
{
    bool bRet = false;

    DPrinterManager *pManager = DPrinterManager::getInstance();
    DDestination *pDest = pManager->getDestinationByName(m_strPrinterName);

    if (pDest != nullptr) {
        if (DESTTYPE::PRINTER == pDest->getType()) {
            DPrinter *pPrinter = static_cast<DPrinter *>(pDest);

            if(pPrinter->isPpdFileBroken()){
                bRet = false;
            }
            else{
                pPrinter->disableSupplys();
                pPrinter->updateSupplys();
                m_supplyInfos.clear();
                m_supplyInfos = pPrinter->getSupplys();

                if(m_supplyInfos.size() > 0){
                    bRet = true;
                }
            }
        }
    }

    return bRet;
}

void DPrinterSupplyShowDlg::updateUI()
{
    initUI();
    initConnection();
}

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

#include "dprintertanslator.h"

DPrinterTanslator::DPrinterTanslator()
{

}

void DPrinterTanslator::init()
{
    m_mapTrans.clear();
    //ColorMode_Combo
    addTranslate("ColorMode_Combo", "Color", tr("Color"));
    addTranslate("ColorMode_Combo", "Grayscale", tr("Grayscale"));
    addTranslate("ColorMode_Combo", "None", tr("Grayscale"));

    //OutputQuanlity_Combo
    addTranslate("OutputQuanlity_Combo", "None", tr("None"));
    addTranslate("OutputQuanlity_Combo", "Draft", tr("Draft"));
    addTranslate("OutputQuanlity_Combo", "Normal", tr("Normal"));

    //PaperOrigin_Combo
    addTranslate("PaperOrigin_Combo", "InputSlot", tr("InputSlot"));
    addTranslate("PaperOrigin_Combo", "Auto-Select", tr("Auto-Select"));
    addTranslate("PaperOrigin_Combo", "Automatic Selection", tr("Auto-Select"));
    addTranslate("PaperOrigin_Combo", "Manual Feeder", tr("Manual Feeder"));
    addTranslate("PaperOrigin_Combo", "Manual Feed", tr("Manual Feeder"));
    addTranslate("PaperOrigin_Combo", "Auto", tr("Auto"));
    addTranslate("PaperOrigin_Combo", "Manual", tr("Manual"));
    addTranslate("PaperOrigin_Combo", "Drawer 1", tr("Drawer 1"));
    addTranslate("PaperOrigin_Combo", "Drawer 2", tr("Drawer 2"));
    addTranslate("PaperOrigin_Combo", "Drawer 3", tr("Drawer 3"));
    addTranslate("PaperOrigin_Combo", "Drawer 4", tr("Drawer 4"));
    addTranslate("PaperOrigin_Combo", "Drawer 5", tr("Drawer 5"));
    addTranslate("PaperOrigin_Combo", "Envelope Feeder", tr("Envelope Feeder"));
    addTranslate("PaperOrigin_Combo", "Tray1", tr("Tray1"));
    addTranslate("PaperOrigin_Combo", "Unknown", tr("Unknown"));

    //PaperType_Combo
    addTranslate("PaperType_Combo", "MediaType", tr("MediaType"));
    addTranslate("PaperType_Combo", "Auto", tr("Auto"));
    addTranslate("PaperType_Combo", "Plain Paper", tr("Plain Paper"));
    addTranslate("PaperType_Combo", "Recycled Paper", tr("Recycled Paper"));
    addTranslate("PaperType_Combo", "Color Paper", tr("Color Paper"));
    addTranslate("PaperType_Combo", "Bond Paper", tr("Bond Paper"));
    addTranslate("PaperType_Combo", "Heavy Paper 1", tr("Heavy Paper 1"));
    addTranslate("PaperType_Combo", "Heavy Paper 2", tr("Heavy Paper 2"));
    addTranslate("PaperType_Combo", "Heavy Paper 3", tr("Heavy Paper 3"));
    addTranslate("PaperType_Combo", "OHP", tr("OHP"));
    addTranslate("PaperType_Combo", "CLEARFILM", tr("OHP"));
    addTranslate("PaperType_Combo", "Labels", tr("Labels"));
    addTranslate("PaperType_Combo", "Envelope", tr("Envelope"));
    addTranslate("PaperType_Combo", "Photo Paper", tr("Photo Paper"));
    addTranslate("PaperType_Combo", "None", tr("None"));

    //PageSize_Combo
    addTranslate("PageSize_Combo", "PageSize", tr("PageSize"));
    addTranslate("PageSize_Combo", "Custom", tr("Custom"));

    //Duplex_Combo
    addTranslate("Duplex_Combo", "Duplex", tr("Duplex"));
    addTranslate("Duplex_Combo", "DuplexTumble", tr("DuplexTumble"));
    addTranslate("Duplex_Combo", "DuplexNoTumble", tr("DuplexNoTumble"));
    addTranslate("Duplex_Combo", "ON (Long-edged Binding)", tr("ON (Long-edged Binding)"));
    addTranslate("Duplex_Combo", "ON (Short-edged Binding)", tr("ON (Short-edged Binding)"));
    addTranslate("Duplex_Combo", "OFF", tr("OFF"));
    addTranslate("Duplex_Combo", "None", tr("OFF"));

    //BindEdge_Combo
    addTranslate("BindEdge_Combo", "BindEdge", tr("BindEdge"));
    addTranslate("BindEdge_Combo", "None", tr("None"));
    addTranslate("BindEdge_Combo", "Left", tr("Left"));
    addTranslate("BindEdge_Combo", "Top", tr("Top"));

    //Orientation_Combo
    addTranslate("Orientation_Combo", "Portrait (no rotation)", tr("Portrait (no rotation)"));
    addTranslate("Orientation_Combo", "Landscape (90 degrees)", tr("Landscape (90 degrees)"));
    addTranslate("Orientation_Combo", "Reverse landscape (270 degrees)", tr("Reverse landscape (270 degrees)"));
    addTranslate("Orientation_Combo", "Reverse portrait (180 degrees)", tr("Reverse portrait (180 degrees)"));
    addTranslate("Orientation_Combo", "Auto", tr("Auto Rotation"));

    //PrintOrder_Combo
    addTranslate("PrintOrder_Combo", "Normal", tr("Normal"));
    addTranslate("PrintOrder_Combo", "Reverse", tr("Reverse"));

    //Finishings_Combo
    addTranslate("Finishings_Combo", "None", tr("None"));
    addTranslate("Finishings_Combo", "Bind", tr("Bind"));
    addTranslate("Finishings_Combo", "Bind(none)", tr("Bind(none)"));
    addTranslate("Finishings_Combo", "Bind (bottom)", tr("Bind (bottom)"));
    addTranslate("Finishings_Combo", "Bind (left)", tr("Bind (left)"));
    addTranslate("Finishings_Combo", "Bind (right)", tr("Bind (right)"));
    addTranslate("Finishings_Combo", "Bind (top)", tr("Bind (top)"));

    //Resolution_Combo
    addTranslate("Resolution_Combo", "None", tr("None"));
}

void DPrinterTanslator::addTranslate(const QString& strContext, const QString& strKey, const QString& strValue)
{
    QMap<QString, QString> mapNode = m_mapTrans.value(strContext);

    if(mapNode.isEmpty()){
        mapNode.insert(strKey, strValue);
        m_mapTrans.insert(strContext, mapNode);
    }
    else {
        mapNode[strKey] = strValue;
        m_mapTrans.insert(strContext, mapNode);
    }
}

QString DPrinterTanslator::translateLocal(const QString & strContext, const QString& strKey)
{
    QMap<QString,QString> mapNode = m_mapTrans.value(strContext);

    if(mapNode.isEmpty())
    {
        return strKey;
    }

    QString strValue = mapNode.value(strKey);
    return strValue.isEmpty() ? strKey : strValue;
}

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
#ifndef DPRINTERPROPERTYTEMPLATE_H
#define DPRINTERPROPERTYTEMPLATE_H
#include <QString>
#include <QVector>
#include <QObject>

#define PROPERTYOPTIONNUM 16

const QString generatePropertyDialogJson(const QVector<QString>& vecOption)
{
    if(vecOption.size() != PROPERTYOPTIONNUM)
    {
        return "";
    }

    return QString::fromStdString("\
    {\
        \"groups\": [\
            {\
                \"key\": \"Print_Setting\",\
                \"name\": \"%1\",\
                \"groups\": [\
                    {\
                        \"key\": \"Driver_Lable\",\
                        \"name\": \"%2\",\
\
                        \"options\": [\
                            {\
                                \"key\": \"Driver_LineEdit\",\
                                \"name\": \"Driver_LineEdit\",\
                                \"type\": \"custom-lineedit\"\
                            }\
                        ]\
                    },\
\
                    {\
                      \"key\": \"URI_Lable\",\
                      \"name\": \"%3\",\
\
                      \"options\": [\
                          {\
                              \"key\": \"URI_LineEdit\",\
                              \"name\": \"URI_LineEdit\",\
                              \"type\": \"custom-lineedit\"\
                          }\
                      ]\
                    },\
\
                    {\
                      \"key\": \"Location_Lable\",\
                      \"name\": \"%4\",\
\
                      \"options\": [\
                          {\
                              \"key\": \"Location_LineEdit\",\
                              \"name\": \"Location_LineEdit\",\
                              \"type\": \"custom-lineedit\"\
                          }\
                      ]\
                    },\
\
                    {\
                      \"key\": \"Description_Lable\",\
                      \"name\": \"%5\",\
\
                      \"options\": [\
                          {\
                              \"key\": \"Description_LineEdit\",\
                              \"name\": \"Description_LineEdit\",\
                              \"type\": \"custom-lineedit\"\
                          }\
                      ]\
                    },\
\
                    {\
                      \"key\": \"ColorMode_Label\",\
                      \"name\": \"%6\",\
                      \"options\": [\
                          {\
                              \"key\": \"ColorMode_Combo\",\
                              \"name\": \"ColorMode_Combo\",\
                              \"type\": \"custom-combobox\"\
                          }\
                      ]\
                    },\
\
                    {\
                      \"key\": \"Resolution_Label\",\
                      \"name\": \"%7\",\
                      \"options\": [\
                          {\
                              \"key\": \"Resolution_Combo\",\
                              \"name\": \"Resolution_Combo\",\
                              \"type\": \"custom-combobox\"\
                          }\
                      ]\
                    },\
\
                    {\
                      \"key\": \"OutputQuanlity_Label\",\
                      \"name\": \"%8\",\
                      \"options\": [\
                          {\
                              \"key\": \"OutputQuanlity_Combo\",\
                              \"name\": \"OutputQuanlity_Combo\",\
                              \"type\": \"custom-combobox\"\
                          }\
                      ]\
                    },\
\
                    {\
                      \"key\": \"PaperOrigin_Label\",\
                      \"name\": \"%9\",\
                      \"options\": [\
                          {\
                              \"key\": \"PaperOrigin_Combo\",\
                              \"name\": \"PaperOrigin_Combo\",\
                              \"type\": \"custom-combobox\"\
                          }\
                      ]\
                    },\
\
                    {\
                      \"key\": \"PaperType_Label\",\
                      \"name\": \"%10\",\
                      \"options\": [\
                          {\
                              \"key\": \"PaperType_Combo\",\
                              \"name\": \"PaperType_Combo\",\
                              \"type\": \"custom-combobox\"\
                          }\
                      ]\
                    },\
\
                    {\
                      \"key\": \"PageSize_Label\",\
                      \"name\": \"%11\",\
                      \"options\": [\
                          {\
                              \"key\": \"PageSize_Combo\",\
                              \"name\": \"PageSize_Combo\",\
                              \"type\": \"custom-combobox\"\
                          }\
                      ]\
                    },\
\
                    {\
                      \"key\": \"DuplexPrint_Label\",\
                      \"name\": \"%12\",\
                      \"options\": [\
                          {\
                              \"key\": \"Duplex_Combo\",\
                              \"name\": \"Duplex_Combo\",\
                              \"type\": \"custom-combobox\"\
                          }\
                      ]\
                    },\
\
                    {\
                      \"key\": \"BindEdge_Label\",\
                      \"name\": \"%13\",\
                      \"options\": [\
                          {\
                              \"key\": \"BindEdge_Combo\",\
                              \"name\": \"BindEdge_Combo\",\
                              \"type\": \"custom-combobox\"\
                          }\
                      ]\
                    },\
\
                    {\
                      \"key\": \"Orientation_Label\",\
                      \"name\": \"%14\",\
                      \"options\": [\
                          {\
                              \"key\": \"Orientation_Combo\",\
                              \"name\": \"Orientation_Combo\",\
                              \"type\": \"custom-combobox\"\
                          }\
                      ]\
                    },\
\
                    {\
                      \"key\": \"PrintOrder_Label\",\
                      \"name\": \"%15\",\
                      \"options\": [\
                          {\
                              \"key\": \"PrintOrder_Combo\",\
                              \"name\": \"PrintOrder_Combo\",\
                              \"type\": \"custom-combobox\"\
                          }\
                      ]\
                    },\
\
                    {\
                        \"key\": \"Finishings\",\
                        \"name\": \"%16\",\
                        \"options\": [\
                            {\
                                \"key\": \"Finishings_Combo\",\
                                \"name\": \"Finishings_Combo\",\
                                \"type\": \"custom-combobox\"\
                            }\
                        ]\
                    }\
                ]\
            }\
        ]\
    }\
                                  ")\
  .arg(vecOption[0]).arg(vecOption[1]).arg(vecOption[2]).arg(vecOption[3]).arg(vecOption[4])\
  .arg(vecOption[5]).arg(vecOption[6]).arg(vecOption[7]).arg(vecOption[8]).arg(vecOption[9])\
   .arg(vecOption[10]).arg(vecOption[11]).arg(vecOption[12]).arg(vecOption[13])\
    .arg(vecOption[14]).arg(vecOption[15]);
}

#endif

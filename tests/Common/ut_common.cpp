/*
 * Copyright (C) 2019 ~ 2020 Uniontech Software Co., Ltd.
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
#include <gtest/gtest.h>

#include <QRegularExpression>

#include "common.h"

class ut_Common : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;
};

void ut_Common::SetUp()
{

}

void ut_Common::TearDown()
{

}

TEST_F(ut_Common, normalize)
{
    /*Epson PM-A820 -> epson pm a 820*/
    QString testMakeModel = "Epson PM-A820";
    QString ret = normalize(testMakeModel);
    QStringList list = ret.split(" ", QString::SkipEmptyParts);
    ASSERT_EQ(list.count(), 4);
    bool flag = ret.contains(QRegularExpression("[A-Z]"));
    ASSERT_EQ(flag, false);
    int number = list.at(3).toInt();
    ASSERT_EQ(number, 820);
}

TEST_F(ut_Common, parseDeviceID)
{
    QString ID = "MFG:Pantum;MDL:Pantum CM9500 Series;";
    auto map = parseDeviceID(ID);
    ASSERT_EQ(map.contains("MFG"), true);
    ASSERT_EQ(map.contains("MDL"), true);
}

TEST_F(ut_Common, shellCmd)
{
    QString cmd = "ls";
    QString out = "";
    QString strError = "";
    int ret = shellCmd(cmd, out, strError);
    ASSERT_EQ(ret, 0);
}

TEST_F(ut_Common, getHostFromUri)
{
    QString uri = "hp:/net/HP_Color_LaserJet_Pro_M252dw?ip=10.0.12.6";
    QString host = getHostFromUri(uri);
    ASSERT_TRUE(host.size() > 0);
}

TEST_F(ut_Common, getPrinterNameFromUri)
{
    QString uri = "hp:/net/HP_Color_LaserJet_Pro_M252dw?ip=10.0.12.6";
    QString name = getPrinterNameFromUri(uri);
    ASSERT_TRUE(name.size() > 0);
}

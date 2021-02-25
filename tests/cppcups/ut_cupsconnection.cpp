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

#include <QDebug>

#include "cupsconnection.h"

class ut_Connection : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    Connection m_cupsConnection;
};

void ut_Connection::SetUp()
{
    /*编译打包环境可能存在cups服务异常的情况*/
    try {
        int ret = m_cupsConnection.init(cupsServer(), ippPort(), cupsEncryption());
        ASSERT_EQ(ret, 0);
    } catch (const std::runtime_error &e) {
        qInfo() << e.what();
    }
}

void ut_Connection::TearDown()
{

}

TEST_F(ut_Connection, getPrinters)
{
    if (m_cupsConnection.http)
        ASSERT_NO_THROW(m_cupsConnection.getPrinters());
    else
        qInfo() << "could not connect to cups server";
}

TEST_F(ut_Connection, getPPDs2)
{
    if (m_cupsConnection.http) {
        auto allPPDS = m_cupsConnection.getPPDs2(0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, -1, nullptr, nullptr, nullptr);
        ASSERT_TRUE(allPPDS.size() > 0);
    } else {
        qInfo() << "could not connect to cups server";
    }

}

TEST_F(ut_Connection, adminGetServerSettings)
{
    if (m_cupsConnection.http) {
        /*某些环境可能没有设置，所以不能根据结果进行测试*/
        ASSERT_NO_THROW(m_cupsConnection.adminGetServerSettings());
    } else {
        qInfo() << "could not connect to cups server";
    }
}

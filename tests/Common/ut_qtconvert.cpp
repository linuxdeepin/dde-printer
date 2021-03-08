/*
 * Copyright (C) 2019 ~ 2020 Uniontech Software Co., Ltd.
 *
 * Author:     liurui <liurui@uniontech.com>
 *
 * Maintainer: liurui <liurui@uniontech.com>
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

#include <QString>

#include "qtconvert.h"

class ut_Qtconvert : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

};

void ut_Qtconvert::SetUp()
{

}

void ut_Qtconvert::TearDown()
{

}

TEST_F(ut_Qtconvert, qStringListStdVector)
{
    QStringList qlist = {"word1", "word2", "word3"};
    vector<string> stdList = qStringListStdVector(qlist);
    ASSERT_EQ(qlist.count(), stdList.size());
}

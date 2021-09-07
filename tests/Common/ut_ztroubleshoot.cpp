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

#include "vendor/ztroubleshoot.h"

class ut_TroubleShoot : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;
    TroubleShoot *m_troubleShoot;
};

void ut_TroubleShoot::SetUp()
{
    m_troubleShoot = new TroubleShoot("5150");
}

void ut_TroubleShoot::TearDown()
{
    delete m_troubleShoot;
    m_troubleShoot = nullptr;
}

TEST_F(ut_TroubleShoot, TroubleShoot)
{
    m_troubleShoot->start();
    sleep(1);
    auto ret = m_troubleShoot->getJobs();
    EXPECT_EQ(ret.size(), 3);
}



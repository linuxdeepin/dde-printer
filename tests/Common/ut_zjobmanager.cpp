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
#include <QDebug>

#include "cupsconnection.h"
#include "zjobmanager.h"

class ut_zjobmanager : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    Connection m_cupsConnection;
};

void ut_zjobmanager::SetUp()
{
    try {
        int ret = m_cupsConnection.init(cupsServer(), ippPort(), cupsEncryption());
        ASSERT_EQ(ret, 0);
    } catch (const std::runtime_error &e) {
        qInfo() << e.what();
    }
}

void ut_zjobmanager::TearDown()
{

}

TEST_F(ut_zjobmanager, getJobs)
{
    map<int, map<string, string>> jobs;
    int ret = g_jobManager->getJobs(jobs, WHICH_JOB_ALL);
    if (m_cupsConnection.http) {
        EXPECT_EQ(ret, 0);
    } else {

        EXPECT_TRUE(jobs.size() == 0);
    }
}

TEST_F(ut_zjobmanager, getJobById)
{
    map<string, string> jobinfo;
    int ret = g_jobManager->getJobById(jobinfo, 80);
    if (m_cupsConnection.http) {
        EXPECT_TRUE(ret == 0 || ret == -2);
    } else {

        EXPECT_TRUE(jobinfo.size() == 0);
    }
}

TEST_F(ut_zjobmanager, cancelJob)
{
    int ret = g_jobManager->cancelJob(1);
    if (m_cupsConnection.http) {
        EXPECT_TRUE(ret <= 0);
    }


}

TEST_F(ut_zjobmanager, deleteJob)
{
    int ret = g_jobManager->deleteJob(0);
    if (m_cupsConnection.http) {
        EXPECT_TRUE(ret <= 0);
    } else {
        EXPECT_EQ(ret, -1);
    }

    ret = g_jobManager->deleteJob(1, "TestPrinter");
    if (m_cupsConnection.http) {
        EXPECT_TRUE(ret <= 0);
    } else {
        EXPECT_EQ(ret, -1);
    }
}

TEST_F(ut_zjobmanager, holdJob)
{
    int ret = g_jobManager->holdJob(0);
    if (m_cupsConnection.http) {
        EXPECT_TRUE(ret <= 0);
    }


}

TEST_F(ut_zjobmanager, holdjobs)
{
    int ret = g_jobManager->holdjobs("TestPrinter");
    if (m_cupsConnection.http) {
        EXPECT_TRUE(ret == -1 || ret == 1);
    }


}

TEST_F(ut_zjobmanager, releaseJob)
{
    int ret = g_jobManager->releaseJob(0);
    if (m_cupsConnection.http) {
        EXPECT_TRUE(ret <= 0);
    }


}

TEST_F(ut_zjobmanager, restartJob)
{
    int ret = g_jobManager->restartJob(0);
    if (m_cupsConnection.http) {
        EXPECT_TRUE(ret <= 0);
    } else {

    }
}

TEST_F(ut_zjobmanager, restartJob1)
{
    int ret = g_jobManager->restartJob(91);
    if (m_cupsConnection.http) {
        EXPECT_TRUE(ret <= 0);
    } else {

    }
}

TEST_F(ut_zjobmanager, moveJob)
{
    char destUri[64] = "ipp://TestPrinter/jobs/100";
    char srcUri[64] = "ipp://TestPrinter1/jobs/TestPrinter1";
    int ret = g_jobManager->moveJob(destUri, 1, nullptr);
    if (m_cupsConnection.http) {
        EXPECT_EQ(ret, -1);
    }

    ret = g_jobManager->moveJob(nullptr, -1, srcUri);
    if (m_cupsConnection.http) {
        EXPECT_EQ(ret, -1);
    }

    ret = g_jobManager->moveJob(nullptr, 1, srcUri);
    if (m_cupsConnection.http) {
        EXPECT_EQ(ret, -1);
    }
}

TEST_F(ut_zjobmanager, priorityJob)
{
    int iPriority = DEFAULT_Priority;
    int ret = g_jobManager->priorityJob(1, iPriority);
    if (m_cupsConnection.http) {
        EXPECT_EQ(ret, -1);
    }

    iPriority = -1;
    ret = g_jobManager->priorityJob(1, iPriority);
    if (m_cupsConnection.http) {
        EXPECT_EQ(ret, -1);
    }
}

TEST_F(ut_zjobmanager, printTestPage)
{
    char printer[16] = "TestPrinter";
    int id = 1;
    QString ret = g_jobManager->printTestPage(printer, id, nullptr);
    if (m_cupsConnection.http) {
        EXPECT_TRUE(ret.size());
    }

}

TEST_F(ut_zjobmanager, isCompletedState)
{
    bool ret = g_jobManager->isCompletedState(IPP_JSTATE_CANCELED);
    EXPECT_TRUE(ret);

    ret = g_jobManager->isCompletedState(IPP_JSTATE_STOPPED);
    EXPECT_FALSE(ret);
}

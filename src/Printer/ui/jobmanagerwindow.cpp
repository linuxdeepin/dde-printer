/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Wei xie <xiewei@deepin.com>
 *
 * Maintainer: Wei xie  <xiewei@deepin.com>
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

#include "jobmanagerwindow.h"
#include "zjobmanager.h"
#include "qtconvert.h"
#include "zcupsmonitor.h"
#include "common.h"

#include <DTitlebar>
#include <DIconButton>
#include <DButtonBox>
#include <DListView>
#include <DDialog>
#include <DPalette>
#include <DApplicationHelper>

#include <QStandardItemModel>
#include <QTableView>
#include <QHeaderView>
#include <QDebug>
#include <QDateTime>
#include <QMenu>
#include <QModelIndexList>
#include <QTimer>
#include <QPixmap>
#include <QRect>
#include <QMouseEvent>
#include <QMargins>
#include <QVBoxLayout>
#include <QLabel>
#include <QToolTip>
#include <QCursor>

#include <cups/ipp.h>
#include <map>
using namespace std;

#define JOB_ACTION_Cancel       0x01
#define JOB_ACTION_Delete       0x02
#define JOB_ACTION_Hold         0x04
#define JOB_ACTION_Release      0x08
#define JOB_ACTION_Priority     0x10
#define JOB_ACTION_Restart      0x20

#define JOB_ACTION_Count        6
#define ACTION_Column           7

#define JOB_VIEW_WIDTH          880
#define ITEM_Height             36
#define ACTION_BUT_SPACE        15

#define JOB_ITEM_ROLE_ID        Qt::UserRole+1
#define JOB_ITEM_ROLE_STATE     Qt::UserRole+2

typedef struct tagHoverAction{

    tagHoverAction() :
        iHoverAction(-1),
        iLastAction(-1)
    {
    }

    QString toString(){
        return QString("last: %1 %2, current: %3 %4").arg(iLastRow).arg(iLastAction).arg(iHoverRow).arg(iHoverAction);
    }

    int iHoverRow;
    int iLastRow;
    int iHoverAction;
    int iLastAction;
}THoverAction;

static THoverAction g_hoverAction;

static QString formatDataTimeString(const QDateTime &dataTime)
{
    QDateTime now = QDateTime::currentDateTime();
    qint64 secs = dataTime.secsTo(now);

    if (secs < 2 * 60) {
        return QObject::tr("a minute ago");
    } else if (secs < 60 * 60) {
        qint64 mins = secs/60;
        return QObject::tr("%1 minutes ago").arg(mins);
    } else if (secs < 24 * 60 * 60) {
        qint64 hours = secs / (60*60);
        if (1 == hours) {
            return QObject::tr("an hour ago");
        } else {
            return QObject::tr("%1 hours ago").arg(hours);
        }
    } else if (secs < 7 * 24 * 60 * 60) {
        qint64 days = secs / (24 * 60 * 60);
        if (1 == days) {
            return QObject::tr("yesterday");
        } else {
            return QObject::tr("%1 days ago").arg(days);
        }
    } else if (secs < 6 * 7 * 24 * 60 * 60) {
        qint64 weeks = secs / (7 * 24 * 60 * 60);
        if (1 == weeks) {
            return QObject::tr("last week");
        } else {
            return QObject::tr("%1 weeks ago").arg(weeks);
        }
    }

    return dataTime.toString("yyyy:MM:dd HH::mm::ss");
}

static QPixmap getActionPixmap(unsigned int iAction, QIcon::Mode mode = QIcon::Normal);
static QPixmap getActionPixmap(unsigned int iAction, QIcon::Mode mode)
{
    QPixmap pixmap;
    QString iconpath;

    switch (iAction) {
    case JOB_ACTION_Cancel:
        iconpath = "dp_job_cancel";
        break;
    case JOB_ACTION_Delete:
        iconpath = "dp_job_delete";
        break;
    case JOB_ACTION_Hold:
        iconpath = "dp_job_pause";
        break;
    case JOB_ACTION_Release:
        iconpath = "dp_job_start";
        break;
    case JOB_ACTION_Restart:
        iconpath = "dp_job_again";
        break;
    case JOB_ACTION_Priority:
        iconpath = "dp_job_priority";
        break;
    default:
        qWarning() << "Unsupport actions: " << iAction;
        break;
    }

    pixmap = QIcon::fromTheme(iconpath).pixmap(QSize(14, 14), mode);

//    qInfo() << iAction << " pixmap size:"<<pixmap.size();
    return pixmap;
}

static QMap<unsigned int, QRect> getItemActionRect(const QRect &itemRect, const QModelIndex &index)
{
    QMap<unsigned int, QRect> actions;
    unsigned int flags = index.data().toUInt();
    int butIndex = 0;

    for (int i=0;i<JOB_ACTION_Count;i++) {
        unsigned int iAction = 1 << i;

        if (flags & iAction) {
            QPixmap pixmap = getActionPixmap(iAction);
            QRect rect(itemRect.left()+ACTION_BUT_SPACE+(pixmap.width()+ACTION_BUT_SPACE)*butIndex++,
                       itemRect.top()+(itemRect.height()-pixmap.height())/2,
                       pixmap.width(), pixmap.height());

            actions.insert(iAction, rect);
        }
    }

    return  actions;
}

JobListView::JobListView(QWidget* parent)
    :QTableView(parent),
    m_contextMenu(nullptr),
    m_itemDelegate(nullptr),
    m_tipsTimer(nullptr)
{
    setAttribute(Qt::WA_Hover);
    setMouseTracking(true);
    setFocusPolicy(Qt::NoFocus);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setShowGrid(false);
    setFrameShape(QFrame::NoFrame);

    m_itemDelegate = new JobItemDelegate();
    setItemDelegate(m_itemDelegate);
    verticalHeader()->setVisible(false);
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader()->setDefaultSectionSize(ITEM_Height);
    horizontalHeader()->setFixedHeight(ITEM_Height);
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

    m_tipsTimer = new QTimer(this);
    m_tipsTimer->setInterval(1000);
    m_tipsTimer->setSingleShot(true);

    connect(m_tipsTimer, &QTimer::timeout, this, &JobListView::slotShowTips);
    connect(this, &QWidget::customContextMenuRequested, this, &JobListView::slotShowContextMenu);

    m_actionNames << tr("Cancel job");
    m_actionNames << tr("Delete job");
    m_actionNames << tr("Hold job");
    m_actionNames << tr("Release job");
    m_actionNames << tr("Priority job");
    m_actionNames << tr("Restart job");

    m_label = new QLabel(tr("No jobs"), this);
    m_label->setAlignment(Qt::AlignCenter);
}

QString JobListView::getActionName(unsigned int iAction)
{
    QString strName;

    for (int i=0;i<JOB_ACTION_Count;i++) {
        if (iAction == static_cast<unsigned int>(1<<i)) {
            strName = m_actionNames[i];
            break;
        }
    }

    return strName;
}

void JobListView::slotShowTips()
{
    QModelIndex index = indexAt(m_tipsPos);
    QRect rect = visualRect(index);
    QMap<unsigned int, QRect> actionRects = getItemActionRect(rect, index);
    QList<unsigned int> flags = actionRects.keys();

    foreach (unsigned int iAction, flags) {
        if (actionRects.value(iAction).contains(m_tipsPos)) {
            QString strTips = getActionName(iAction);

            QToolTip::showText(mapToGlobal(m_tipsPos), strTips, this);
        }
    }
}

void JobListView::mouseMoveEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    QRect rect = visualRect(index);
    QMap<unsigned int, QRect> actionRects = getItemActionRect(rect, index);
    QList<unsigned int> flags = actionRects.keys();
    int iRow = index.row(), action = -1;

    QTableView::mouseMoveEvent(event);

    //查找是否有按钮处于hover状态
    foreach (unsigned int iAction, flags) {
        if (actionRects.value(iAction).contains(event->pos())) {
            action = static_cast<int>(iAction);
            m_tipsPos = event->pos();
            m_tipsTimer->start();
        }
    }

//    qInfo() << iRow << action;

    //如果有按钮处于hover状态
    if (-1 != action) {
        //如果还是原来按钮，状态不变
        if (iRow == g_hoverAction.iHoverRow && action == g_hoverAction.iHoverAction) {
            return;
        }

        QModelIndex lastIndex;
        QModelIndex curIndex;
        //如果切换了按钮,刷新两个按钮的状态
        if (-1 != g_hoverAction.iHoverAction) {
            g_hoverAction.iLastRow = g_hoverAction.iHoverRow;
            g_hoverAction.iLastAction = g_hoverAction.iHoverAction;

            lastIndex = model()->index(g_hoverAction.iLastRow, ACTION_Column);

        //如果没有切换按钮，只用刷新一个按钮
        } else {
            lastIndex = model()->index(iRow, ACTION_Column);
        }

        g_hoverAction.iHoverRow = iRow;
        g_hoverAction.iHoverAction = action;
        curIndex = model()->index(g_hoverAction.iHoverRow, ACTION_Column);

//        qInfo() << "Update index to hover: " << g_hoverAction.toString();
//        qInfo() << "Update current index: " << curIndex;
//        qInfo() << "Update last index: " << lastIndex;

        if (lastIndex.row() > curIndex.row())
            dataChanged(curIndex, lastIndex);
        else
            dataChanged(lastIndex, curIndex);

    //如果没有按钮处于hover状态，将之前hover的按钮设置成normal
    } else if (-1 != g_hoverAction.iHoverAction){
        g_hoverAction.iLastRow = g_hoverAction.iHoverRow;
        g_hoverAction.iLastAction = g_hoverAction.iHoverAction;
        g_hoverAction.iHoverAction = -1;

        QModelIndex lastIndex;
        lastIndex = model()->index(g_hoverAction.iLastRow, ACTION_Column);
//        qInfo() << "Set index to normal: " << g_hoverAction.toString();
//        qInfo() << "Set index to normal: " << lastIndex;
        dataChanged(lastIndex, lastIndex);

        m_tipsTimer->stop();
    }
}

void JobListView::mouseReleaseEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    QRect rect = visualRect(index);
    QMap<unsigned int, QRect> actionRects = getItemActionRect(rect, index);
    QList<unsigned int> flags = actionRects.keys();

    QTableView::mouseReleaseEvent(event);

    foreach (unsigned int iAction, flags) {
        if (actionRects.value(iAction).contains(event->pos()) &&
             askDeleteJobs(iAction)) {
            doItemAction(getJobId(index.row()), iAction);
        }
    }
}

void JobListView::resizeEvent(QResizeEvent *event)
{
    int headerHeight = horizontalHeader()->height();
    QTableView::resizeEvent(event);

    m_label->setGeometry(0, headerHeight, event->size().width(), event->size().height()-headerHeight);
}

void JobListView::setLabelContentVisable(bool bShow)
{
    m_label->setHidden(!bShow);
}

int JobListView::getJobId(int iRow)
{
    QModelIndex idIndex = model()->index(iRow, 0);
    return idIndex.data().toString().toInt();
}

bool JobListView::askDeleteJobs(unsigned int flag)
{
    int iAccept;
    DDialog dlg("", tr("Are you sure to delete job"), this);

    if (JOB_ACTION_Delete != flag)
        return true;

    dlg.setIcon(QIcon(":/images/warning_logo.svg"));
    dlg.addButton(tr("Cancel"), true);
    iAccept = dlg.addButton(tr("Delete job"), false, DDialog::ButtonWarning);
    dlg.setContentsMargins(10, 15, 10, 15);
    dlg.setModal(true);
    dlg.setFixedSize(422, 202);

    return (iAccept == dlg.exec());
}

void JobListView::doItemAction(int jobId, unsigned int iAction)
{
    switch (iAction) {
    case JOB_ACTION_Cancel:
        g_jobManager->cancelJob(jobId);
        break;
    case JOB_ACTION_Delete:
            g_jobManager->deleteJob(jobId);
        break;
    case JOB_ACTION_Hold:
        g_jobManager->holdJob(jobId);
        break;
    case JOB_ACTION_Release:
        g_jobManager->releaseJob(jobId);
        break;
    case JOB_ACTION_Restart:
        g_jobManager->restartJob(jobId);
        break;
    case JOB_ACTION_Priority:
    {
        JobsDataModel *jobModel = static_cast<JobsDataModel*>(model());
        int iPriority = jobModel->getHighestPriority()+1;

        if (HIGHEST_Priority <= iPriority){
            iPriority = -1;
        }

        if (0 == g_jobManager->priorityJob(jobId, iPriority))
            jobModel->setHighestPriority(iPriority);

        break;
    }
    default:
        qWarning() << "Unsupport actions: " << iAction;
        break;
    }
}

void JobListView::slotShowContextMenu(const QPoint &pos)
{
    QModelIndexList selectList = selectionModel()->selectedRows(0);
    unsigned int flag = 0;
    JobsDataModel *jobsModel = static_cast<JobsDataModel*>(model());

    if (selectList.isEmpty()) return;

    foreach (QModelIndex index, selectList) {
        flag |= jobsModel->getActionStatus(index.row());
    }

    if (!m_contextMenu) {
        m_contextMenu = new QMenu(this);
        for (int i=0;i<JOB_ACTION_Count;i++) {
            m_atctions.append(m_contextMenu->addAction(m_actionNames[i]));
        }

        connect(m_contextMenu, &QMenu::triggered, this, &JobListView::slotMenuTriggered);
    }

    for(int i=0;i<m_atctions.count();i++) {
        m_atctions[i]->setEnabled(flag & (1<<i));
    }

    m_contextMenu->exec(mapToGlobal(pos));
}

void JobListView::processAction(int index)
{
    unsigned int flag = 1<<index;
    QModelIndexList selectList = selectionModel()->selectedRows(0);
    JobsDataModel *jobsModel = static_cast<JobsDataModel*>(model());

    if (selectList.isEmpty() || !askDeleteJobs(flag))
        return;

    foreach (auto modelindex, selectList) {
        unsigned int itemflag = jobsModel->getActionStatus(modelindex.row());
        int jobId = jobsModel->getJobId(modelindex.row());

        if (itemflag&flag) {
            doItemAction(jobId, flag);
        }
    }
}

void JobListView::slotMenuTriggered(QAction *action)
{
    int index = m_atctions.indexOf(action);
    qDebug() << index;
    processAction(index);
}

JobItemDelegate::JobItemDelegate(QObject* parent)
    :QItemDelegate (parent)
{}

void JobItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QMap<unsigned int, QRect> actions;
    QList<unsigned int> flags;

//    qInfo() << index;

    if (index.row() % 2) {
        DPalette pl(DApplicationHelper::instance()->palette(option.widget));

        painter->fillRect(option.rect, pl.brush(QPalette::AlternateBase));
    }

    if (index.column() != ACTION_Column) {
        QStyleOptionViewItem newoption = option;
        int iState = index.data(JOB_ITEM_ROLE_STATE).toInt();

        if (IPP_JSTATE_ABORTED == iState || IPP_JSTATE_STOPPED == iState) {
            newoption.palette.setColor(QPalette::Inactive, QPalette::Text, QColor(255,87,54,100));
        }
        QItemDelegate::paint(painter, newoption, index);
        return;
    }

    actions = getItemActionRect(option.rect, index);
    flags = actions.keys();
    foreach (unsigned int iAction, flags) {
        QPixmap pixmap;

        if (g_hoverAction.iHoverAction == static_cast<int>(iAction) &&
            g_hoverAction.iHoverRow == index.row()) {
            pixmap = getActionPixmap(iAction, QIcon::Active);
        } else {
            pixmap = getActionPixmap(iAction);
        }

        painter->drawPixmap(actions.value(iAction), pixmap);
    }
}

QSize JobItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    int iRow = index.row();

//    qInfo() << ITEM_Height;

    if (0 == iRow)
        return QSize(72, ITEM_Height);
    else if(1 == iRow)
        return QSize(102, ITEM_Height);
    else if(2 == iRow)
        return QSize(137, ITEM_Height);
    else if(3 == iRow)
        return QSize(131, ITEM_Height);
    else if(4 == iRow)
        return QSize(65, ITEM_Height);
    else if(5 == iRow)
        return QSize(124, ITEM_Height);
    else if(6 == iRow)
        return QSize(146, ITEM_Height);

    return option.rect.size();
}

JobsDataModel::JobsDataModel(QObject* parent)
    : QAbstractTableModel(parent),
      m_iHighestPriority(51),
    m_iWhichJob(WHICH_JOB_RUNING)
{
    m_reflushTimer = new QTimer(this);
    m_reflushTimer->setInterval(100);
    m_reflushTimer->setSingleShot(true);
    connect(m_reflushTimer, &QTimer::timeout, this, &JobsDataModel::slotReflushJobItems);
}

QVariant JobsDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        if (section == 0) {
            return tr("id");
        } else if (section == 1) {
            return tr("user");
        } else if (section == 2) {
            return tr("job name");
        } else if (section == 3) {
            return tr("printer name");
        } else if (section == 4) {
            return tr("size");
        } else if (section == 5) {
            return tr("create time");
        } else if (section == 6) {
            return tr("job state");
        } else if (section == 7) {
            return tr("action");
        }
    }
    return QVariant();
}

void JobsDataModel::sortJobs()
{
    QList<QMap<QString, QVariant>> donelist, processinglist, stoplist;
    QMap<QString, QVariant> priorityJob;
    QMap<int, QMap<QString, QVariant>> jobHash;

    foreach (auto job, m_jobs) {
        int iState = job[JOB_ATTR_STATE].toString().toInt();
        int jobPriority = job[JOB_ATTR_PRIORITY].toString().toInt();

        if (IPP_JSTATE_PENDING == iState) {
            jobHash.insertMulti(jobPriority, job);
        } else if (iState == IPP_JSTATE_PROCESSING) {
            processinglist.prepend(job);
        } else if (iState == IPP_JSTATE_HELD || IPP_JSTATE_STOPPED == iState) {
            stoplist.prepend(job);
        } else {
            donelist.prepend(job);
        }
    }

    m_jobs.clear();
    m_jobs = processinglist;
    foreach (auto job, jobHash.values()) {
        m_jobs.insert(processinglist.count(), job);
    }
    m_jobs += stoplist;
    m_jobs += donelist;
}

void JobsDataModel::setWhichJob(int which)
{
    m_iWhichJob = which;

    slotReflushJobsList();
}

void JobsDataModel::updateJobState(int id, int state, const QString &message)
{
    int index = 0;
    int iState = 0;
    Q_UNUSED(message);

    for (;index<m_jobs.count();index++) {
        if (id == m_jobs[index][JOB_ATTR_ID].toInt()) {
            iState = m_jobs[index][JOB_ATTR_STATE].toInt();
            break;
        }
    }

    qInfo() << iState << m_iWhichJob;

    if (index >= m_jobs.count()) {
        //如果未完成任务不在任务列表中,而且当前需要显示未完成任务，则将新任务添加到任务列表中
        //如果已完成任务不在任务列表中,而且当前只显示未完成任务，则将已完成任务添加到列表中
        if (((IPP_JSTATE_PROCESSING <= state && m_iWhichJob != WHICH_JOB_DONE) ||
            (IPP_JSTATE_PROCESSING > state && m_iWhichJob == WHICH_JOB_DONE))) {
            map<string, string> jobinfo;
            if (0 == g_jobManager->getJobById(jobinfo, id)) {
                QMap<QString, QVariant> job;
                map<string, string>::const_iterator itjob;

                job.insert(JOB_ATTR_ID, id);
                for (itjob=jobinfo.begin();itjob!=jobinfo.end();itjob++) {
                    job.insert(STQ(itjob->first), attrValueToQString(itjob->second));
                }
                m_jobs.append(job);
                slotReflushJobItems();
                return;
            }
        }

        //其他不在任务列表的情况不需要刷新列表
        return;
    }


    //如果已完成任务在任务列表中，而且当前只显示未完成任务，则将已完成任务从任务列表中删除
    if (IPP_JSTATE_PROCESSING < state && m_iWhichJob == WHICH_JOB_RUNING) {
        m_jobs.removeAt(index);
        slotReflushJobItems();
        return;
    }

    //其他情况只用刷新任务状态
    if (iState != state) {
        map<string, string> jobinfo;
        QMap<QString, QVariant> job;
        if (0 == g_jobManager->getJobById(jobinfo, id)) {
            map<string, string>::const_iterator itjob;

            job.insert(JOB_ATTR_ID, id);
            for (itjob=jobinfo.begin();itjob!=jobinfo.end();itjob++) {
                job.insert(STQ(itjob->first), attrValueToQString(itjob->second));
            }
        } else {
            job = m_jobs[index];
            job.insert(JOB_ATTR_STATE, state);
        }
        m_jobs.removeAt(index);
        m_jobs.insert(index, job);
    }
    //如果刷新的定时器已经启动，和定时器事件一起刷新
    if (!m_reflushTimer->isActive())
        dataChanged(this->index(index, 0), this->index(index, ACTION_Column));
}

void JobsDataModel::slotReflushJobItems()
{
    int jobPriority, iState;
    int priorityCount = 0;

    m_reflushTimer->stop();
    beginResetModel();

    //获取最高优先级的值，用于列表显示任务是否能优先打印
    m_iHighestPriority = 51;
    for (int i=0;i<m_jobs.count();i++) {
        jobPriority = m_jobs[i][JOB_ATTR_PRIORITY].toString().toInt();
        iState = m_jobs[i][JOB_ATTR_STATE].toString().toInt();

        if (jobPriority > m_iHighestPriority && iState <= IPP_JSTATE_PROCESSING) {
            m_iHighestPriority = jobPriority;
            priorityCount++;
        } else if(jobPriority == m_iHighestPriority && iState <= IPP_JSTATE_PROCESSING){
            priorityCount++;
        }
    }
    //如果多个任务都是优先打印，将最高优先级加1保证都可以再次设置优先打印
    if (priorityCount > 1)
        m_iHighestPriority++;

    sortJobs();

    endResetModel();
    emit signalJobsCountChanged(m_jobs.count());
    qInfo() << "Current highest priorty" << m_iHighestPriority;
}

void JobsDataModel::slotReflushJobsList()
{
    map<int, map<string, string>> jobsmap;
    map<int, map<string, string>>::const_iterator itmaps;

    qInfo() << m_iWhichJob;

    if (0 != g_jobManager->getJobs(jobsmap, m_iWhichJob))
        return;

    m_iHighestPriority = 51;
    m_jobs.clear();
    for (itmaps=jobsmap.begin();itmaps!=jobsmap.end();itmaps++) {
        QMap<QString, QVariant> job;
        map<string, string> jobinfo = itmaps->second;
        map<string, string>::const_iterator itjob;

        job.insert(JOB_ATTR_ID, itmaps->first);
        for (itjob=jobinfo.begin();itjob!=jobinfo.end();itjob++) {
            job.insert(STQ(itjob->first), attrValueToQString(itjob->second));
        }
        m_jobs.append(job);
    }

    slotReflushJobItems();
}

int JobsDataModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_jobs.size();
}

int JobsDataModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 8;
}

QVariant JobsDataModel::data(const QModelIndex &index, int role) const
{
    int iRow = index.row();
    QMap<QString, QVariant> job = m_jobs[iRow];

    if (!index.isValid() || role != Qt::DisplayRole) {
        if (JOB_ITEM_ROLE_ID == role) {
            return job[JOB_ATTR_ID];
        } else if (JOB_ITEM_ROLE_STATE == role){
            return job[JOB_ATTR_STATE];
        }
        return QVariant();
    }

    if (index.column() == 0) {
        return job[JOB_ATTR_ID].toInt();
    } else if (index.column() == 1) {
        return job[JOB_ATTR_USER].toString();
    } else if (index.column() == 2) {
        return job[JOB_ATTR_NAME].toString();
    } else if (index.column() == 3) {
        QString uri = job[JOB_ATTR_URI].toString();
        return getPrinterNameFromUri(uri);
    } else if (index.column() == 4) {
        QString strSize = job[JOB_ATTR_SIZE].toString();
        int iSize = strSize.toInt();

        if (iSize > 1024) {
            iSize /= 1024;
            strSize = QString::number(iSize) + "M";
        } else {
            strSize += "K";
        }

        return strSize;
    } else if (index.column() == 5) {
        QString strTime = job[JOB_ATTR_TIME_ADD].toString();
        unsigned int iTime = strTime.toUInt();
        QDateTime dateTime = QDateTime::fromSecsSinceEpoch(iTime);

        return formatDataTimeString(dateTime);
    } else if (index.column() == 6) {
        return g_cupsMonitor->getJobNotify(job);
    } else if (index.column() == 7) {
        return getActionStatus(iRow);
    }
    return QVariant();
}

int JobsDataModel::getHighestPriority()
{
    return m_iHighestPriority;
}

void JobsDataModel::setHighestPriority(int iPriority)
{
    m_iHighestPriority = iPriority;
}

unsigned int JobsDataModel::getActionStatus(int iRow) const
{
    unsigned int flag = 0;
    QMap<QString, QVariant> job = m_jobs[iRow];
    int iState = job[JOB_ATTR_STATE].toString().toInt();

    switch (iState) {
    case IPP_JSTATE_PENDING:
    {
        int iPriority = job[JOB_ATTR_PRIORITY].toString().toInt();

        if (iPriority < m_iHighestPriority) {
            flag = JOB_ACTION_Priority;
        }

        Q_FALLTHROUGH();
    }
    case IPP_JSTATE_PROCESSING:
        flag |= JOB_ACTION_Cancel|JOB_ACTION_Hold;
        break;
    case IPP_JSTATE_HELD:
        flag = JOB_ACTION_Cancel|JOB_ACTION_Release;
        break;
    case IPP_JSTATE_STOPPED:
    case IPP_JSTATE_CANCELED:
    case IPP_JSTATE_ABORTED:
    case IPP_JSTATE_COMPLETED:
        flag = JOB_ACTION_Delete|JOB_ACTION_Restart;
        break;
    default:
        flag = JOB_ACTION_Delete;
        break;
    }

    return  flag;
}

int JobsDataModel::getJobId(int iRow)
{
    return m_jobs[iRow][JOB_ATTR_ID].toInt();
}

JobManagerWindow::JobManagerWindow(QWidget* parent)
    : DMainWindow (parent),
      m_jobsView(nullptr),
      m_jobsModel(nullptr),
      m_reflushBut(nullptr),
      m_whichButBox(nullptr),
      m_jobCountLabel(nullptr)
{
    createUi();
    initUi();
    initConnect();
    m_jobsModel->setWhichJob(WHICH_JOB_RUNING);
}

void JobManagerWindow::createUi()
{
    m_reflushBut = new DIconButton(titlebar());
    m_whichButBox = new DButtonBox(titlebar());
    m_whichList.append(new DButtonBoxButton(QIcon::fromTheme("dp_print_all")));
    m_whichList.append(new DButtonBoxButton(QIcon::fromTheme("dp_print_queue")));
    m_whichList.append(new DButtonBoxButton(QIcon::fromTheme("dp_print_done")));

    m_jobsView = new JobListView(this);
    m_jobsModel = new JobsDataModel();

    m_jobCountLabel = new QLabel(this);
}

void JobManagerWindow::initUi()
{
    m_reflushBut->setIcon(QIcon::fromTheme("dp_refresh"));
    m_reflushBut->setToolTip(tr("Reflush"));

    m_whichButBox->setButtonList(m_whichList, true);
    m_whichList[WHICH_JOB_RUNING]->setChecked(true);
    m_whichList[WHICH_JOB_ALL]->setToolTip(tr("All jobs"));
    m_whichList[WHICH_JOB_RUNING]->setToolTip(tr("Runing jobs"));
    m_whichList[WHICH_JOB_DONE]->setToolTip(tr("Finished jobs"));
    foreach (DButtonBoxButton* but, m_whichList) {
        but->setFocusPolicy(Qt::NoFocus);
    }

    titlebar()->addWidget(m_whichButBox, Qt::AlignLeft);
    titlebar()->addWidget(m_reflushBut, Qt::AlignLeft);
    titlebar()->setIcon(QIcon(":/images/dde-printer.svg"));
    titlebar()->setTitle("");
    titlebar()->setMenuVisible(false);

    m_jobsView->setModel(m_jobsModel);
    m_jobsView->horizontalHeader()->resizeSection(0, 72);
    m_jobsView->horizontalHeader()->resizeSection(1, 82);
    m_jobsView->horizontalHeader()->resizeSection(2, 137);
    m_jobsView->horizontalHeader()->resizeSection(3, 131);
    m_jobsView->horizontalHeader()->resizeSection(4, 65);
    m_jobsView->horizontalHeader()->resizeSection(5, 124);
    m_jobsView->horizontalHeader()->resizeSection(6, 146);

    m_jobCountLabel->setAlignment(Qt::AlignCenter);

    setMinimumSize(JOB_VIEW_WIDTH, 546);
    takeCentralWidget();

    QWidget* centWidget = new QWidget(this);
    QVBoxLayout *lay = new QVBoxLayout();
    lay->addWidget(m_jobsView);
    lay->addWidget(m_jobCountLabel);
    lay->setContentsMargins(10, 10, 10, 10);
    centWidget->setLayout(lay);
    setCentralWidget(centWidget);
}

void JobManagerWindow::initConnect()
{
    connect(m_reflushBut, &QAbstractButton::clicked, m_jobsModel, &JobsDataModel::slotReflushJobsList);
    connect(m_whichButBox, &DButtonBox::buttonClicked, this, &JobManagerWindow::slotWhichBoxClicked);
    connect(g_cupsMonitor, &CupsMonitor::signalJobStateChanged, this, &JobManagerWindow::slotJobStateChanged);
    connect(m_jobsModel, &JobsDataModel::signalJobsCountChanged, this, &JobManagerWindow::slotJobsCountChanged);
}

void JobManagerWindow::slotWhichBoxClicked(QAbstractButton *whichbut)
{
    if (whichbut) {
        int iWhichJob = m_whichList.indexOf(static_cast<DButtonBoxButton*>(whichbut));

        m_jobsModel->setWhichJob(iWhichJob);
    }
}

void JobManagerWindow::slotJobsCountChanged(int count)
{
    qInfo() << count;
    m_jobCountLabel->setText(tr("%1 jobs").arg(count));
    m_jobsView->setLabelContentVisable(0==count);
}

void JobManagerWindow::slotJobStateChanged(int id, int state, const QString &message)
{
    m_jobsModel->updateJobState(id, state, message);
}

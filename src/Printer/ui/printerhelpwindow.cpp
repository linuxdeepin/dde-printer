// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "printerhelpwindow.h"
#include "qtconvert.h"

#include <DTitlebar>
#include <DFrame>
#include <DFontSizeManager>

#ifdef DTKWIDGET_CLASS_DSizeMode
#include <DSizeMode>
#endif

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QClipboard>
#include <QScrollArea>
#include <QDesktopServices>
#include <QUrl>

#define UI_PRINTER_PROBLEM_ENTRANCE QObject::tr("Instructions for Use")
#define UI_PRINTER_ADD_HELP QObject::tr("Help on adding and using printers")
#define UI_PRINTER_PROBLEM_GUIDANCE QObject::tr("Can't print? Learn how to resolve")

static const QString faqDocPath = "/usr/share/deepin-manual/manual-assets/application/dde-printer/打印机使用FAQ文档.pdf";

#define REMOVE_CUPS_TEMP "sudo systemctl stop cups\n" \
    "sudo rm -rf /var/spool/cups/temp/*\n" \
    "sudo systemctl start cups"

static const QList<QPair<QString, QString>> helpContent = {
    {QObject::tr("1. Check printer connection"), QObject::tr("Please make sure the printer is turned on and properly connected to this computer. Please check if the printer power, USB or network connection is normal.")},
    {QObject::tr("2. Check the printer status"), QObject::tr("Please make sure that the printer is working normally, for example, there are no paper jams, ink cartridge exhaustion, etc.")},
    {QObject::tr("3. Check the printer driver"), QObject::tr("Make sure the printer driver is installed correctly. If the driver does not match the printer, it is recommended to try updating the driver or download the latest version of the driver from the printer manufacturer's official website.")},
    {QObject::tr("4. Restart the print service"), QObject::tr("Open System Monitor, switch to System Services, search for \"cups\" and try to restart the service.")},
    {QObject::tr("5. Check the print queue"), QObject::tr("In the print manager, find the printer in use and click the \"Print Queue\" icon button. Check for any pending print tasks, delete any pending tasks, and then try printing again.")},
    {QObject::tr("6. Run troubleshooting"), QObject::tr("In the print manager, find the printer in use, click the \"Troubleshooting\" icon button, run it and find solutions based on the detection results.")},
    {QObject::tr("7. Check the default printer settings"), QObject::tr("In Print Manager, find the printer you are using and confirm whether it is set as default.")},
    {QObject::tr("8. Clean up temporary printer files"), QObject::tr("Open the terminal and enter the following commands in sequence and press Enter. This will stop the print service, delete the printer's temporary files, and finally restart the print service.")},
    {QObject::tr("9. Update your print manager or operating system"), QObject::tr("Make sure your print manager or operating system is up to date. You can install the latest version of Print Manager through the App Store, and install the latest version of UOS through the Control Center-Update Module.")},
    {QObject::tr("10. Reinstall the printer"), QObject::tr("In Print Manager, select the printer you are using, click the \"Minus\" button to remove it, and then try to reinstall the printer.")},
    {QObject::tr("11. Check antivirus software"), QObject::tr("Some antivirus software may block printing operations. Try turning off your antivirus software and retesting the printing functionality.")},
    {QObject::tr("12. Check for hardware failure"), QObject::tr("If none of the above steps solve the problem, you need to consider whether the printer has a hardware failure. It is recommended to try connecting to another computer to test, or contact the manufacturer's technical support.")}
};

PrinterHelpWindow::PrinterHelpWindow(QWidget *parent)
    : DMainWindow(parent)
{
    initUi();
    initConnections();
}

PrinterHelpWindow::~PrinterHelpWindow()
{

}

void PrinterHelpWindow::initUi()
{
    titlebar()->setMenuVisible(false);
    titlebar()->setTitle("");
    titlebar()->setIcon(QIcon(":/images/dde-printer.svg"));

    setWindowFlags(Qt::Dialog);
    setWindowModality(Qt::ApplicationModal);
    setFixedSize(760, 600);

    DFrame *pWidget = new DFrame();
    pWidget->setFrameShape(DFrame::Shape::NoFrame);
    pWidget->setAutoFillBackground(true);

    QLabel *titlelabel = new QLabel();
    titlelabel->setText(tr("Learn how to fix issues with not being able to print"));
    titlelabel->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(titlelabel, DFontSizeManager::T6, int(QFont::Normal));
    titlelabel->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *pVLayout = new QVBoxLayout();
    pVLayout->setContentsMargins(60, 10, 60, 10);

    pVLayout->addWidget(titlelabel);

    auto it = helpContent.begin();
    for (int i = 1; it != helpContent.end(); ++i, ++it) {
        QLabel *tmpTitle = new QLabel;
        QLabel *content = new QLabel;
        DFontSizeManager::instance()->bind(tmpTitle, DFontSizeManager::T6, int(QFont::Normal));
        DFontSizeManager::instance()->bind(content, DFontSizeManager::T7, int(QFont::Light));
        tmpTitle->setText(QObject::tr(it->first.toUtf8().data()));
        content->setText(QObject::tr(it->second.toUtf8().data()));
        content->setWordWrap(true);

        pVLayout->addWidget(tmpTitle);
        pVLayout->addWidget(content);

        if (i == 8) {
           m_copyLabel = new QLabel;
           DFontSizeManager::instance()->bind(m_copyLabel, DFontSizeManager::T7, int(QFont::Light));
           m_copyLabel->setText(REMOVE_CUPS_TEMP);

           m_copyLabel->setContentsMargins(10, 10, 10, 10);
           m_copyLabel->setAutoFillBackground(true);
           m_copyLabel->setWordWrap(true);

           QHBoxLayout *pHcopyLayout = new QHBoxLayout();
           m_copyButton = new QPushButton(tr("Copy"));
           m_copyButton->setFlat(true);
           DFontSizeManager::instance()->bind(m_copyButton, DFontSizeManager::T8, int(QFont::Light));
           QPalette palette1;
           palette1.setColor(QPalette::ButtonText, "#0081FF");
           m_copyButton->setPalette(palette1);

           pHcopyLayout->addWidget(m_copyLabel);
           pHcopyLayout->addWidget(m_copyButton, 0, Qt::AlignRight | Qt::AlignTop);

           QWidget *pInfoWidget = new QWidget();
           pInfoWidget->setLayout(pHcopyLayout);
           QPalette palette = pInfoWidget->palette();
           palette.setColor(QPalette::Window, QColor(0, 0, 112));
           pInfoWidget->setPalette(palette);
           pVLayout->addWidget(pInfoWidget);
        }
    }

    pWidget->setLayout(pVLayout);

    QScrollArea *docScrollArea = new QScrollArea;
    docScrollArea->setContentsMargins(0, 0, 0, 0);
    docScrollArea->setWidgetResizable(true);
    docScrollArea->setFrameShape(QFrame::Shape::NoFrame);
    docScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
    docScrollArea->setWidget(pWidget);
    QPalette pa1 = docScrollArea->palette();
    pa1.setBrush(QPalette::Window, Qt::transparent);
    docScrollArea->setPalette(pa1);

    takeCentralWidget();
    setCentralWidget(docScrollArea);
}

void PrinterHelpWindow::initConnections()
{
    QObject::connect(m_copyButton, &QPushButton::clicked, [&]() {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(m_copyLabel->text());
    });
}

void CustomLabel::initUi()
{
    this->installEventFilter(this);
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover, true);
    QLabel *imageLabel = new QLabel(this);
    QIcon icon(QIcon::fromTheme("icon_tips"));
    imageLabel->setPixmap(icon.pixmap(QSize(20, 20)));
    QPalette palette;
    palette.setColor(QPalette::WindowText, "#0081FF");
    imageLabel->setPalette(palette);
    imageLabel->setContentsMargins(0, 0, 0, 0);

    m_textLabel = new QLabel(this);
    m_textLabel->setText(UI_PRINTER_PROBLEM_ENTRANCE);

    m_main = new QMenu(this);
    m_main->setFixedSize(200, 88);
    QString tipInfo = UI_PRINTER_ADD_HELP;
    m_popupButton1 = new QPushButton(m_main);
    m_popupButton1->setFixedWidth(180);
    m_popupButton1->setToolTip(UI_PRINTER_ADD_HELP);
    m_popupButton1->setFlat(true);
    DFontSizeManager::instance()->bind(m_popupButton1, DFontSizeManager::T6, QFont::Normal);

    QString tipInfo2 = UI_PRINTER_PROBLEM_GUIDANCE;
    m_popupButton2 = new QPushButton(UI_PRINTER_PROBLEM_GUIDANCE);
    m_popupButton2->setToolTip(UI_PRINTER_PROBLEM_GUIDANCE);
    m_popupButton2->setFixedWidth(180);
    DFontSizeManager::instance()->bind(m_popupButton2, DFontSizeManager::T6, QFont::Normal);
    m_popupButton2->setFlat(true);

    m_main->installEventFilter(this);
    m_main->setMouseTracking(true);
    m_main->setContentsMargins(0, 0, 0, 0);
    m_main->setAttribute(Qt::WA_Hover, true);

    geteElidedText(m_popupButton1->font(), tipInfo, m_popupButton1->width() - 20);
    m_popupButton1->setText(tipInfo);
    geteElidedText(m_popupButton2->font(), tipInfo2, m_popupButton2->width() - 20);
    m_popupButton2->setText(tipInfo2);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_popupButton1);
    layout->addWidget(m_popupButton2);
    m_main->setLayout(layout);

    QHBoxLayout *hLayout = new QHBoxLayout;

    hLayout->addWidget(m_textLabel, Qt::AlignRight);
    hLayout->addWidget(imageLabel, Qt::AlignRight);
    hLayout->setContentsMargins(0, 0, 0, 0);

    QFontMetrics fontWidth(m_textLabel->font());
    int width = fontWidth.horizontalAdvance(m_textLabel->text());
    setMinimumWidth(width + 30);
    setMaximumWidth(500);
    setLayout(hLayout);
    setAlignment(Qt::AlignRight);
}

void CustomLabel::initConnection()
{
    connect(m_popupButton1, &QPushButton::clicked, this, &CustomLabel::slotOpenFaqDoc);
    connect(m_popupButton2, &QPushButton::clicked, this, &CustomLabel::slotOpenHelpInfo);
}

void CustomLabel::initSubUi()
{
    QPoint globalPos = this->mapToGlobal(QPoint(0, 0));
    m_main->move(QPoint(globalPos.x() + this->width() - m_main->width(),  globalPos.y() - m_main->height()));
    m_main->show();
}

void CustomLabel::slotOpenFaqDoc()
{
    m_main->hide();
    QUrl url = QUrl::fromLocalFile(faqDocPath);
    QDesktopServices::openUrl(url);
}

void CustomLabel::slotOpenHelpInfo()
{
    if (!m_pHelpWindow) {
        m_pHelpWindow = new PrinterHelpWindow(this);
    }
    m_pHelpWindow->show();
    m_main->hide();
}

void CustomLabel::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::FontChange) {
        QString tipInfo1 = UI_PRINTER_ADD_HELP;
        geteElidedText(m_popupButton1->font(), tipInfo1, m_popupButton1->width() - 20);
        m_popupButton1->setText(tipInfo1);

        QString tipInfo2 = UI_PRINTER_PROBLEM_GUIDANCE;
        geteElidedText(m_popupButton2->font(), tipInfo2, m_popupButton2->width() - 20);
        m_popupButton2->setText(tipInfo2);
        QWidget::changeEvent(event);
    }
}

bool CustomLabel::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_main && event->type() == QEvent::HoverLeave) {
        m_main->hide();
        return true;
    } else if (watched == this && event->type() == QEvent::HoverEnter) {
        initSubUi();
        return true;
    } else if (watched == m_main && event->type() == QEvent::HoverMove) {
        QPoint curpos = QCursor::pos();
        QPoint labelPos = this->mapToGlobal(QPoint(0, 0));
        QRect labelRect(labelPos, this->size());
        QPoint menuPos = m_main->mapToGlobal(QPoint(0, 0));
        QRect menuRect(menuPos, m_main->size());
        if (!(labelRect.contains(curpos) || menuRect.contains(curpos))) {
            m_main->hide();
            return true;
        }
    }
    return QLabel::eventFilter(watched, event);
}

QT += core gui network dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = dde-printer
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS QT_MESSAGELOGCONTEXT

CONFIG += c++11 link_pkgconfig
PKGCONFIG += dtkwidget dtkgui

SOURCES += \
        main.cpp \
    ui/dprintersshowwindow.cpp \
    util/dprintermanager.cpp \
    util/dprinter.cpp \
    ui/printersearchwindow.cpp \
    util/dprintclass.cpp \
    util/ddestination.cpp \ 
    vendor/common.cpp \
    vendor/ztaskinterface.cpp \
    vendor/zdevicemanager.cpp \
    vendor/addprinter.cpp \
    vendor/zcupsmonitor.cpp \
    vendor/ztroubleshoot.cpp \
    vendor/zjobmanager.cpp \
    vendor/zdrivermanager.cpp \
    vendor/qtconvert.cpp \
    vendor/printerservice.cpp \
    ui/dpropertysetdlg.cpp \
    ui/installdriverwindow.cpp \
    ui/renameprinterwindow.cpp \
    ui/jobmanagerwindow.cpp \
    ui/printerapplication.cpp \
    ui/installprinterwindow.cpp \
    ui/permissionswindow.cpp \
    util/connectedtask.cpp \
    vendor/zsettings.cpp \
    ui/printertestpagedialog.cpp \
    ui/troubleshootdialog.cpp\
    util/dprintertanslator.cpp


RESOURCES +=         resources.qrc \
    icons/theme-icons.qrc

FORMS +=

HEADERS += \
    ui/dprintersshowwindow.h \
    util/dprintermanager.h \
    util/dprinter.h \
    ui/printersearchwindow.h \
    util/ddestination.h \
    util/dprintclass.h \   
    vendor/qtconvert.h \
    vendor/zdrivermanager_p.h \
    vendor/zcupsmonitor.h \
    vendor/zdevicemanager.h \
    vendor/ztroubleshoot.h \
    vendor/common.h \
    vendor/zjobmanager.h \
    vendor/ztaskinterface.h \
    vendor/addprinter.h \
    vendor/ztroubleshoot_p.h \
    vendor/cupsattrnames.h \
    vendor/zdrivermanager.h \
    vendor/config.h \
    vendor/addprinter_p.h \
    vendor/printerservice.h \
    vendor/cupsattrnames.h \
    ui/dpropertysetdlg.h \
    ui/installdriverwindow.h \
    ui/renameprinterwindow.h \
    ui/jobmanagerwindow.h \
    ui/printerapplication.h \
    ui/installprinterwindow.h \
    ui/uisourcestring.h \
    ui/permissionswindow.h \
    util/connectedtask.h \
    vendor/zsettings.h \
    ui/printertestpagedialog.h \
    ui/troubleshootdialog.h\
    util/dprintertanslator.h \
    ui/dprinterpropertytemplate.h


INCLUDEPATH +=  \
                $$PWD/../cppcups \
                vendor \
                /usr/include/samba-4.0/ \
                ui \
                util
DEPENDPATH += $$PWD/../cppcups

QMAKE_CFLAGS += -Wall -Wextra -Wformat=2 -Wno-format-nonliteral -Wshadow
QMAKE_CXXFLAGS += -Wall -Wextra -Wformat=2 -Wno-format-nonliteral -Wshadow


QMAKE_LFLAGS += -Wl,-rpath=../cppcups

unix:!macx:{
LIBS += -L../cppcups/ -lcppcups
LIBS += -lsmbclient -lcups -lcrypto++
}

DISTFILES +=

TRANSLATIONS  +=  translations/dde-printer_zh_CN.ts \
                  translations/dde-printer_en_AU.ts

CONFIG(release, debug|release) {
    !system($$PWD/translate_generation.sh): error("Failed to generate translation")
}

linux {

isEmpty(PREFIX){
    PREFIX = /usr
}

target.path = $${PREFIX}/bin

watch.path = /etc/xdg/autostart
watch.files = $${PWD}/platform/linux/dde-printer-watch.desktop

desktop.path = $${PREFIX}/share/applications
desktop.files = $${PWD}/platform/linux/dde-printer.desktop

hicolor.path =  $${PREFIX}/share/icons/hicolor/48x48/apps
hicolor.files = $${PWD}/images/dde-printer.svg

trans.path =  $${PREFIX}/share/dde-printer/translations
trans.files = $${PWD}/translations/*.qm

polkit.path = $${PREFIX}/share/polkit-1/actions
polkit.files = $${PWD}/policy/com.deepin.pkexec.devPrinter.policy

keyring.path =  $${PREFIX}/share/keyrings
keyring.files = $${PWD}/ppa/printer-keyring.gpg

gpgfile.path =  $${PREFIX}/share/dde-printer
gpgfile.files = $${PWD}/ppa/printer.gpg

#ppa.path =  /etc/apt/sources.list.d
#ppa.files = $${PWD}/ppa/printer.list

INSTALLS += target watch desktop hicolor trans polkit keyring gpgfile ppa
}

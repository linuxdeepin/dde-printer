QT += core dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = dde-printer-helper
TEMPLATE = app
CONFIG += c++11 link_pkgconfig
PKGCONFIG += dtkwidget dtkgui

SOURCES += \
        main.cpp \
    dbus/zcupsmonitor.cpp

RESOURCES +=         resources.qrc

INCLUDEPATH +=  \
                $$PWD/../cppcups \
                $$PWD/../Common

DEPENDPATH += $$PWD/../cppcups

QMAKE_CFLAGS += -Wall -Wextra -Wformat=2 -Wno-format-nonliteral -Wshadow
QMAKE_CXXFLAGS += -Wall -Wextra -Wformat=2 -Wno-format-nonliteral -Wshadow

unix:!macx:{
LIBS += -L../cppcups/ -l:libcppcups.a
LIBS += -lcups
}

HEADERS += \
    dbus/zcupsmonitor.h

DISTFILES +=
linux {
target.path = $${PREFIX}/bin

watch.path = /etc/xdg/autostart
watch.files = $${PWD}/platform/linux/watch/dde-printer-watch.desktop
INSTALLS += target watch
}
include(../Common/Common.pri)

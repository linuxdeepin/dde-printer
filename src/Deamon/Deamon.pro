QT += core dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = dde-printer-helper
TEMPLATE = app
CONFIG += c++11 link_pkgconfig
PKGCONFIG += dtkwidget dtkgui

SOURCES += \
        main.cpp \
    dbus/zcupsmonitor.cpp \
    dbus/helperinterface.cpp \
    usbprinter/usbthread.cpp \
    usbprinter/signalforwarder.cpp

RESOURCES +=         resources.qrc

INCLUDEPATH +=  \
                $$PWD/../cppcups \
                $$PWD/../Common \
                $$PWD/../Common/vendor

DESTDIR += $$PWD

DEPENDPATH += $$PWD/../cppcups

QMAKE_CFLAGS += -Wall -Wextra -Wformat=2 -Wno-format-nonliteral -Wshadow -fstack-protector-strong -D_FORTITY_SOURCE=1 -fPIC
QMAKE_CXXFLAGS += -Wall -Wextra -Wformat=2 -Wno-format-nonliteral -Wshadow -fstack-protector-strong -D_FORTITY_SOURCE=1 -fPIC
QMAKE_LFLAGS += -z noexecstack -pie -z lazy

unix:!macx:{
LIBS += -L$$PWD/../cppcups/ -lcppcups
LIBS += -lcups -lusb-1.0
}

HEADERS += \
    dbus/zcupsmonitor.h \
    dbus/helperinterface.h \
    usbprinter/usbthread.h \
    usbprinter/signalforwarder.h

DISTFILES +=
linux {
isEmpty(PREFIX){
    PREFIX = /usr
}

target.path = $${PREFIX}/bin

watch.path = /etc/xdg/autostart
watch.files = $${PWD}/platform/linux/watch/dde-printer-watch.desktop

trans.path =  $${PREFIX}/share/dde-printer-helper/translations
trans.files = $${PWD}/translations/*.qm

INSTALLS += target watch trans
}
include(../Common/Common.pri)

TRANSLATIONS  +=  translations/dde-printer-helper.ts

CONFIG(release, debug|release) {
    !system($$PWD/translate_generation.sh): error("Failed to generate translation")
}


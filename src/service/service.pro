DEFINES += QT_MESSAGELOGCONTEXT

QT += core dbus
QT -= gui

CONFIG += c++11

TARGET = dde-printer-manager
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    service.cpp

DEFINES += QT_DEPRECATED_WARNINGS

HEADERS += \
    service.h

unix {
isEmpty(PREFIX) {
    PREFIX = /usr
}
}

binary.path = $${PREFIX}/lib/deepin-daemon
binary.files = $${OUT_PWD}/$${TARGET}

service.path = $${PREFIX}/share/dbus-1/system-services
service.files = $$PWD/data/com.deepin.printer.manager.service

dbus.path = /etc/dbus-1/system.d
dbus.files = $$PWD/data/com.deepin.printer.manager.conf

INSTALLS += service dbus binary

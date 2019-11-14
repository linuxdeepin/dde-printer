#-------------------------------------------------
#
# Project created by QtCreator 2019-10-11T11:14:07
#
#-------------------------------------------------

QT       -= core gui

#CONFIG += link_pkgconfig
#PKGCONFIG += cups
LIBS += -lcups

TARGET = cppcups
TEMPLATE = lib

DEFINES += CPPCUPS_LIBRARY

SOURCES += cupsconnection.cc\
    cupsipp.cc\
    cupsmodule.cc\
    cupsppd.cc

HEADERS += cupsconnection.h \
        cppcups_global.h\
        cupsipp.h \
        cupsmodule.h\
        cupsppd.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

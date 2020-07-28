#-------------------------------------------------
#
# Project created by QtCreator 2019-10-11T11:14:07
#
#-------------------------------------------------

QT       -= core gui
CONFIG += staticlib

TARGET = cppcups
TEMPLATE = lib

DEFINES += CPPCUPS_LIBRARY

SOURCES += cupsconnection.cc\
    cupsipp.cc\
    cupsmodule.cc\
    cupsppd.cc \
    cupssnmp.cpp

HEADERS += cupsconnection.h \
        cppcups_global.h\
        cupsipp.h \
        cupsmodule.h\
        cupsppd.h \
    cupssnmp.h

INCLUDEPATH += $${PWD}/cups_private

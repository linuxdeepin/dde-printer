#-------------------------------------------------
#
# Project created by QtCreator 2017-06-26T14:20:36
#
#-------------------------------------------------

TEMPLATE = subdirs
SUBDIRS += \
src/cppcups/cppcups.pro \
src/Printer/Printer.pro \
    src/Deamon

CONFIG += ordered
DEFINES += QT_MESSAGELOGCONTEXT

#debian.files = $$PWD/debian/*
#debian.path = $$OUT_PWD/dde-printer/debian/

#INSTALLS +=debian

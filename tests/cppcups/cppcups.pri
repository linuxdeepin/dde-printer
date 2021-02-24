INCLUDEPATH += $$PWD/../../src/cppcups

unix:LIBS += -L$$PWD/../../src/cppcups -lcppcups

SOURCES += $$PWD/ut_cupsconnection.cpp

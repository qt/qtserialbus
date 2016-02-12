QT = core serialbus

TARGET = canbusutil
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    readtask.cpp \
    canbusutil.cpp \
    sigtermhandler.cpp

HEADERS += \
    readtask.h \
    canbusutil.h \
    sigtermhandler.h

load(qt_tool)

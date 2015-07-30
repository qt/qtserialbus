QT = core serialbus

TARGET = canbusutil
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

load(qt_tool)

SOURCES += main.cpp \
    readtask.cpp \
    canbusutil.cpp \
    sigtermhandler.cpp

HEADERS += \
    readtask.h \
    canbusutil.h \
    sigtermhandler.h

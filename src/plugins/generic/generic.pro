QT += core-private serialbus

TARGET = qtbusgeneric

PLUGIN_TYPE = canbus
PLUGIN_EXTENDS = serialbus
PLUGIN_CLASS_NAME = DummyBusPlugin
load(qt_plugin)

HEADERS += \
    dummybackend.h

SOURCES += main.cpp \
    dummybackend.cpp

OTHER_FILES = plugin.json

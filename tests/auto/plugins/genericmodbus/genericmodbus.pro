QT += core-private serialbus

TARGET = qtmodbustestgeneric

PLUGIN_TYPE = modbus
PLUGIN_EXTENDS = serialbus
PLUGIN_CLASS_NAME = DummyBusPlugin
load(qt_plugin)

HEADERS += \
    dummyslave.h \
    dummymaster.h

SOURCES += main.cpp \
    dummyslave.cpp \
    dummymaster.cpp

OTHER_FILES = plugin.json

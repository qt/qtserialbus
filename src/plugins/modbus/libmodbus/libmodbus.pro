QT += core-private serialbus serialport

TARGET = qtmodbus

CONFIG += link_pkgconfig
PKGCONFIG += libmodbus

PLUGIN_TYPE = modbus
PLUGIN_EXTENDS = serialbus
PLUGIN_CLASS_NAME = LibModBusPlugin
load(qt_plugin)

HEADERS += \
    libmodbusslave.h \
    libmodbusmaster.h \
    libmodbusreply.h

SOURCES += main.cpp \
    libmodbusslave.cpp \
    libmodbusmaster.cpp \
    libmodbusreply.cpp

OTHER_FILES = plugin.json

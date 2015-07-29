QT += core-private serialbus serialport

TARGET = qtmodbus

CONFIG += link_pkgconfig
PKGCONFIG += libmodbus

PLUGIN_TYPE = modbus
PLUGIN_EXTENDS = serialbus
PLUGIN_CLASS_NAME = LibModBusPlugin
load(qt_plugin)

HEADERS += \
    libmodbusbackend.h

SOURCES += main.cpp \
    libmodbusbackend.cpp

OTHER_FILES = plugin.json

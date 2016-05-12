TARGET = qttinycanbus

QT = core-private serialbus

PUBLIC_HEADERS += \
    tinycanbackend.h

PRIVATE_HEADERS += \
    tinycanbackend_p.h \
    tinycan_symbols_p.h

SOURCES += main.cpp \
    tinycanbackend.cpp

DISTFILES = plugin.json

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS

PLUGIN_TYPE = canbus
PLUGIN_EXTENDS = serialbus
PLUGIN_CLASS_NAME = TinyCanBusPlugin
load(qt_plugin)

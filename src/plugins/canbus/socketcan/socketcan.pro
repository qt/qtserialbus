QT += core-private serialbus

TARGET = qtsocketcanbus

PLUGIN_TYPE = canbus
PLUGIN_EXTENDS = serialbus
PLUGIN_CLASS_NAME = SocketCanBusPlugin
load(qt_plugin)

HEADERS += \
    socketcanbackend.h \

SOURCES += main.cpp \
    socketcanbackend.cpp \

OTHER_FILES = plugin.json

QT += core-private serialbus

TARGET = qtcanbus

PLUGIN_TYPE = serialbuses
PLUGIN_EXTENDS = serialbus
PLUGIN_CLASS_NAME = SocketCanBusPlugin
load(qt_plugin)

HEADERS += \
    socketcanbackend.h \

SOURCES += main.cpp \
    socketcanbackend.cpp \

OTHER_FILES = plugin.json

TARGET = qtsocketcanbus

QT = core serialbus

HEADERS += \
    libsocketcan.h \
    socketcanbackend.h

SOURCES += \
    libsocketcan.cpp \
    main.cpp \
    socketcanbackend.cpp

DISTFILES = plugin.json

PLUGIN_TYPE = canbus
PLUGIN_CLASS_NAME = SocketCanBusPlugin
load(qt_plugin)

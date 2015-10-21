QT += core-private serialbus network

TARGET = qtmodbus

PLUGIN_TYPE = modbus
PLUGIN_EXTENDS = serialbus
PLUGIN_CLASS_NAME = LibModBusPlugin
load(qt_plugin)

# Use system lib version if available
config_libmodbus_systemlib {
    CONFIG += link_pkgconfig
    PKGCONFIG += libmodbus
} else {
    INCLUDEPATH += $$PWD/../../../3rdparty/libmodbus
    win32:include($$PWD/../../../3rdparty/libmodbus/libmodbus_win.pri)
    LIBS_PRIVATE += -L$$MODULE_BASE_OUTDIR/lib -lmodbus$$qtPlatformTargetSuffix()
}

HEADERS += \
    libmodbusslave.h \
    libmodbusmaster.h \
    libmodbusreply.h \
    libmodbustcpclient.h

SOURCES += main.cpp \
    libmodbusslave.cpp \
    libmodbusmaster.cpp \
    libmodbusreply.cpp \
    libmodbustcpclient.cpp

OTHER_FILES = plugin.json

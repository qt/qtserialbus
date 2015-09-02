QT += core-private serialbus

TARGET = qtpeakcanbus

PLUGIN_TYPE = canbus
PLUGIN_EXTENDS = serialbus
PLUGIN_CLASS_NAME = PeakCanBusPlugin
load(qt_plugin)

PUBLIC_HEADERS += \
    peakcanbackend.h

PRIVATE_HEADERS += \
    peakcanbackend_p.h \
    peakcan_symbols_p.h

SOURCES += \
    main.cpp \
    peakcanbackend.cpp

OTHER_FILES = plugin.json

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS

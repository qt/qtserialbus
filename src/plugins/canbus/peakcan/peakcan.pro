TARGET = qtpeakcanbus

QT = core-private serialbus

PUBLIC_HEADERS += \
    peakcanbackend.h

PRIVATE_HEADERS += \
    peakcanbackend_p.h \
    peakcan_symbols_p.h

SOURCES += \
    main.cpp \
    peakcanbackend.cpp

DISTFILES = plugin.json

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS

PLUGIN_TYPE = canbus
PLUGIN_EXTENDS = serialbus
PLUGIN_CLASS_NAME = PeakCanBusPlugin
load(qt_plugin)

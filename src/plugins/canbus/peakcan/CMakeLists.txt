# Generated from peakcan.pro.

#####################################################################
## qtpeakcanbus Plugin:
#####################################################################

add_qt_plugin(qtpeakcanbus
    TYPE canbus
    CLASS_NAME PeakCanBusPlugin
    SOURCES
        main.cpp
        peakcan_symbols_p.h
        peakcanbackend.cpp peakcanbackend.h peakcanbackend_p.h
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::SerialBus
)

#### Keys ignored in scope 1:.:.:peakcan.pro:<TRUE>:
# DISTFILES = "plugin.json"
# _LOADED = "qt_plugin"

#####################################################################
## PeakCanBusPlugin Plugin:
#####################################################################

qt_internal_add_plugin(PeakCanBusPlugin
    OUTPUT_NAME qtpeakcanbus
    PLUGIN_TYPE canbus
    SOURCES
        main.cpp
        peakcan_symbols_p.h
        peakcanbackend.cpp peakcanbackend.h peakcanbackend_p.h
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::SerialBus
)

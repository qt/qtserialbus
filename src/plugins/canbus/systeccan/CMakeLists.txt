#####################################################################
## SystecCanBusPlugin Plugin:
#####################################################################

qt_internal_add_plugin(SystecCanBusPlugin
    OUTPUT_NAME qtsysteccanbus
    PLUGIN_TYPE canbus
    SOURCES
        main.cpp
        systeccan_symbols_p.h
        systeccanbackend.cpp systeccanbackend.h systeccanbackend_p.h
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Network
        Qt::SerialBus
)

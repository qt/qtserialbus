#####################################################################
## VectorCanBusPlugin Plugin:
#####################################################################

qt_internal_add_plugin(VectorCanBusPlugin
    OUTPUT_NAME qtvectorcanbus
    PLUGIN_TYPE canbus
    SOURCES
        main.cpp
        vectorcan_symbols_p.h
        vectorcanbackend.cpp vectorcanbackend.h vectorcanbackend_p.h
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::SerialBus
)

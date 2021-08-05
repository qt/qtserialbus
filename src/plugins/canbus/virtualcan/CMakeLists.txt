#####################################################################
## VirtualCanBusPlugin Plugin:
#####################################################################

qt_internal_add_plugin(VirtualCanBusPlugin
    OUTPUT_NAME qtvirtualcanbus
    PLUGIN_TYPE canbus
    SOURCES
        main.cpp
        virtualcanbackend.cpp virtualcanbackend.h
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::SerialBus
)

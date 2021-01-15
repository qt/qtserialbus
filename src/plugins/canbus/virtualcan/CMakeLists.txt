#####################################################################
## VirtualCanBusPlugin Plugin:
#####################################################################

qt_internal_add_plugin(VirtualCanBusPlugin
    OUTPUT_NAME qtvirtualcanbus
    TYPE canbus
    SOURCES
        main.cpp
        virtualcanbackend.cpp virtualcanbackend.h
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Network
        Qt::SerialBus
)

#####################################################################
## TestCanBusPlugin Plugin:
#####################################################################

qt_internal_add_plugin(TestCanBusPlugin
    OUTPUT_NAME qttestcanbus
    PLUGIN_TYPE canbus
    DEFAULT_IF FALSE
    STATIC
    SOURCES
        testcanbackend.cpp testcanbackend.h
        main.cpp
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Network
        Qt::SerialBus
)

#####################################################################
## GenericBusPlugin Plugin:
#####################################################################

qt_internal_add_plugin(GenericBusPlugin
    OUTPUT_NAME qtcanbustestgeneric
    TYPE canbus
    DEFAULT_IF FALSE
    STATIC
    SOURCES
        dummybackend.cpp dummybackend.h
        main.cpp
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Network
        Qt::SerialBus
)

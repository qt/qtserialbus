#####################################################################
## GenericBusPluginV1 Plugin:
#####################################################################

qt_internal_add_plugin(GenericBusPluginV1
    OUTPUT_NAME qtcanbustestgenericv1
    TYPE canbus
    DEFAULT_IF FALSE
    STATIC
    SOURCES
        dummybackendv1.cpp dummybackendv1.h
        main.cpp
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::SerialBus
)

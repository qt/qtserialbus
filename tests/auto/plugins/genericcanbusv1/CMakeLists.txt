# Generated from genericcanbusv1.pro.

#####################################################################
## qtcanbustestgenericv1 Plugin:
#####################################################################

qt_add_plugin(qtcanbustestgenericv1
    TYPE canbus
    CLASS_NAME GenericBusPluginV1
    SOURCES
        dummybackendv1.cpp dummybackendv1.h
        main.cpp
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::SerialBus
)

#### Keys ignored in scope 1:.:.:genericcanbusv1.pro:<TRUE>:
# DISTFILES = "plugin.json"
# PLUGIN_EXTENDS = "-"

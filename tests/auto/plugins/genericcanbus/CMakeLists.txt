# Generated from genericcanbus.pro.

#####################################################################
## qtcanbustestgeneric Plugin:
#####################################################################

qt_add_plugin(qtcanbustestgeneric
    TYPE canbus
    CLASS_NAME GenericBusPlugin
    SOURCES
        dummybackend.cpp dummybackend.h
        main.cpp
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::SerialBus
)

#### Keys ignored in scope 1:.:.:genericcanbus.pro:<TRUE>:
# DISTFILES = "plugin.json"
# PLUGIN_EXTENDS = "-"

# Generated from tinycan.pro.

#####################################################################
## qttinycanbus Plugin:
#####################################################################

add_qt_plugin(qttinycanbus
    TYPE canbus
    CLASS_NAME TinyCanBusPlugin
    SOURCES
        main.cpp
        tinycan_symbols_p.h
        tinycanbackend.cpp tinycanbackend.h tinycanbackend_p.h
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::SerialBus
)

#### Keys ignored in scope 1:.:.:tinycan.pro:<TRUE>:
# DISTFILES = "plugin.json"
# _LOADED = "qt_plugin"

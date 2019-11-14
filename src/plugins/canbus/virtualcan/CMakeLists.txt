# Generated from virtualcan.pro.

#####################################################################
## qtvirtualcanbus Plugin:
#####################################################################

qt_add_plugin(qtvirtualcanbus
    TYPE canbus
    CLASS_NAME VirtualCanBusPlugin
    SOURCES
        main.cpp
        virtualcanbackend.cpp virtualcanbackend.h
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Network
        Qt::SerialBus
)

#### Keys ignored in scope 1:.:.:virtualcan.pro:<TRUE>:
# DISTFILES = "plugin.json"

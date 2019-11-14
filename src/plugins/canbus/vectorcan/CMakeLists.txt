# Generated from vectorcan.pro.

#####################################################################
## qtvectorcanbus Plugin:
#####################################################################

qt_add_plugin(qtvectorcanbus
    TYPE canbus
    CLASS_NAME VectorCanBusPlugin
    SOURCES
        main.cpp
        vectorcan_symbols_p.h
        vectorcanbackend.cpp vectorcanbackend.h vectorcanbackend_p.h
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::SerialBus
)

#### Keys ignored in scope 1:.:.:vectorcan.pro:<TRUE>:
# DISTFILES = "plugin.json"

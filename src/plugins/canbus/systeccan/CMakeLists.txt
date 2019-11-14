# Generated from systeccan.pro.

#####################################################################
## qtsysteccanbus Plugin:
#####################################################################

qt_add_plugin(qtsysteccanbus
    TYPE canbus
    CLASS_NAME SystecCanBusPlugin
    SOURCES
        main.cpp
        systeccan_symbols_p.h
        systeccanbackend.cpp systeccanbackend.h systeccanbackend_p.h
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::SerialBus
)

#### Keys ignored in scope 1:.:.:systeccan.pro:<TRUE>:
# DISTFILES = "plugin.json"

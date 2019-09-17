# Generated from socketcan.pro.

#####################################################################
## qtsocketcanbus Plugin:
#####################################################################

add_qt_plugin(qtsocketcanbus
    TYPE canbus
    CLASS_NAME SocketCanBusPlugin
    SOURCES
        libsocketcan.cpp libsocketcan.h
        main.cpp
        socketcanbackend.cpp socketcanbackend.h
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::SerialBus
)

#### Keys ignored in scope 1:.:.:socketcan.pro:<TRUE>:
# DISTFILES = "plugin.json"
# _LOADED = "qt_plugin"

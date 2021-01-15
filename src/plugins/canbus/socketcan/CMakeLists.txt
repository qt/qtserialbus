#####################################################################
## SocketCanBusPlugin Plugin:
#####################################################################

qt_internal_add_plugin(SocketCanBusPlugin
    OUTPUT_NAME qtsocketcanbus
    TYPE canbus
    SOURCES
        libsocketcan.cpp libsocketcan.h
        main.cpp
        socketcanbackend.cpp socketcanbackend.h
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::SerialBus
)

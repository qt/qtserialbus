#####################################################################
## TinyCanBusPlugin Plugin:
#####################################################################

qt_internal_add_plugin(TinyCanBusPlugin
    OUTPUT_NAME qttinycanbus
    TYPE canbus
    SOURCES
        main.cpp
        tinycan_symbols_p.h
        tinycanbackend.cpp tinycanbackend.h tinycanbackend_p.h
    LIBRARIES
        Qt::Core
        Qt::SerialBus
)

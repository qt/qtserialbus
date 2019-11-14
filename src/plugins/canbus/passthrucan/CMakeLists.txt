# Generated from passthrucan.pro.

#####################################################################
## qtpassthrucanbus Plugin:
#####################################################################

qt_add_plugin(qtpassthrucanbus
    TYPE canbus
    CLASS_NAME PassThruCanBusPlugin
    SOURCES
        j2534passthru.cpp j2534passthru.h
        main.cpp
        passthrucanbackend.cpp passthrucanbackend.h
        passthrucanio.cpp passthrucanio.h
    PUBLIC_LIBRARIES
        Qt::SerialBus
)

#### Keys ignored in scope 1:.:.:passthrucan.pro:<TRUE>:
# DISTFILES = "plugin.json"

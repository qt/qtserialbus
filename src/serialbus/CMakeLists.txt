# Generated from serialbus.pro.

#####################################################################
## SerialBus Module:
#####################################################################

add_qt_module(SerialBus
    PLUGIN_TYPES canbus
    SOURCES
        qcanbus.cpp qcanbus.h
        qcanbusdevice.cpp qcanbusdevice.h qcanbusdevice_p.h
        qcanbusdeviceinfo.cpp qcanbusdeviceinfo.h qcanbusdeviceinfo_p.h
        qcanbusfactory.cpp qcanbusfactory.h
        qcanbusframe.cpp qcanbusframe.h
        qmodbus_symbols_p.h
        qmodbusadu_p.h
        qmodbusclient.cpp qmodbusclient.h qmodbusclient_p.h
        qmodbuscommevent_p.h
        qmodbusdataunit.cpp qmodbusdataunit.h
        qmodbusdevice.cpp qmodbusdevice.h qmodbusdevice_p.h
        qmodbusdeviceidentification.cpp qmodbusdeviceidentification.h
        qmodbuspdu.cpp qmodbuspdu.h
        qmodbusreply.cpp qmodbusreply.h
        qmodbusserver.cpp qmodbusserver.h qmodbusserver_p.h
        qmodbustcpclient.cpp qmodbustcpclient.h qmodbustcpclient_p.h
        qmodbustcpserver.cpp qmodbustcpserver.h qmodbustcpserver_p.h
        qtserialbusglobal.h
    LIBRARIES
        Qt::CorePrivate
        Qt::Network
    PUBLIC_LIBRARIES
        Qt::Core
)

#### Keys ignored in scope 1:.:.:serialbus.pro:<TRUE>:
# _LOADED = "qt_module"

## Scopes:
#####################################################################

extend_target(SerialBus CONDITION QT_FEATURE_modbus_serialport
    SOURCES
        qmodbusrtuserialmaster.cpp
        qmodbusrtuserialslave.cpp
    LIBRARIES
        Qt::SerialPort
)

#### Keys ignored in scope 2:.:.:serialbus.pro:QT_FEATURE_modbus_serialport:
# PRIVATE_HEADERS = "qmodbusrtuserialmaster_p.h" "qmodbusrtuserialslave_p.h"
# PUBLIC_HEADERS = "qmodbusrtuserialmaster.h" "qmodbusrtuserialslave.h"
add_qt_docs(SerialBus
    doc/qtserialbus.qdocconf
)


TARGET = QtSerialBus

QT = core-private
QT_FOR_PRIVATE = network

CONFIG += c++11

QMAKE_DOCS = $$PWD/doc/qtserialbus.qdocconf

PUBLIC_HEADERS += \
    qcanbusdevice.h \
    qcanbusdeviceinfo.h \
    qcanbusfactory.h \
    qcanbusframe.h \
    qcanbus.h \
    qtserialbusglobal.h \
    qmodbusserver.h \
    qmodbusdevice.h \
    qmodbusdataunit.h \
    qmodbusclient.h \
    qmodbusreply.h \
    qmodbustcpclient.h \
    qmodbustcpserver.h \
    qmodbuspdu.h \
    qmodbusdeviceidentification.h

PRIVATE_HEADERS += \
    qcanbusdevice_p.h \
    qcanbusdeviceinfo_p.h \
    qmodbusserver_p.h \
    qmodbusclient_p.h \
    qmodbusdevice_p.h \
    qmodbustcpclient_p.h \
    qmodbustcpserver_p.h \
    qmodbus_symbols_p.h \
    qmodbuscommevent_p.h \
    qmodbusadu_p.h \

SOURCES += \
    qcanbusdevice.cpp \
    qcanbusdeviceinfo.cpp \
    qcanbus.cpp \
    qcanbusfactory.cpp \
    qcanbusframe.cpp \
    qmodbusserver.cpp \
    qmodbusdevice.cpp \
    qmodbusdataunit.cpp \
    qmodbusclient.cpp \
    qmodbusreply.cpp \
    qmodbustcpclient.cpp \
    qmodbustcpserver.cpp \
    qmodbuspdu.cpp \
    qmodbusdeviceidentification.cpp

qtConfig(modbus-serialport) {
    QT_FOR_PRIVATE += serialport

    PUBLIC_HEADERS += \
        qmodbusrtuserialmaster.h \
        qmodbusrtuserialslave.h

    PRIVATE_HEADERS += \
        qmodbusrtuserialmaster_p.h \
        qmodbusrtuserialslave_p.h

    SOURCES += \
        qmodbusrtuserialmaster.cpp \
        qmodbusrtuserialslave.cpp
}
HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS

MODULE_PLUGIN_TYPES = \
    canbus
load(qt_module)

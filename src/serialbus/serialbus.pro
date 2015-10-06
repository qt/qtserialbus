TARGET = QtSerialBus
MODULE = serialbus
QT += core-private network
CONFIG += c++11

QMAKE_DOCS = $$PWD/doc/qtserialbus.qdocconf

MODULE_PLUGIN_TYPES = \
    serialbuses

load(qt_module)

PUBLIC_HEADERS += \
    qcanbusdevice.h \
    qcanbusframe.h \
    qcanbus.h \
    qserialbusglobal.h \
    qcanbusfactory.h \
    qmodbus.h \
    qmodbusserver.h \
    qmodbusfactory.h \
    qmodbusdevice.h \
    qmodbusdataunit.h \
    qmodbusclient.h \
    qmodbusreply.h \
    qmodbusregister.h \
    qmodbusrtuserialmaster.h \
    qmodbustcpclient.h \
    qmodbustcpserver.h \
    qmodbusrtuserialslave.h \
    qmodbuspdu.h \
    qmodbusreplyex.h

PRIVATE_HEADERS += \
    qcanbusdevice_p.h \
    qmodbusserver_p.h \
    qmodbusclient_p.h \
    qmodbusdevice_p.h \
    qmodbusrtuserialmaster_p.h \
    qmodbustcpclient_p.h \
    qmodbustcpserver_p.h \
    qmodbusrtuserialslave_p.h

SOURCES += \
    qcanbusdevice.cpp \
    qcanbus.cpp \
    qcanbusfactory.cpp \
    qcanbusframe.cpp \
    qmodbus.cpp \
    qmodbusserver.cpp \
    qmodbusdevice.cpp \
    qmodbusdataunit.cpp \
    qmodbusclient.cpp \
    qmodbusreply.cpp \
    qmodbusregister.cpp \
    qmodbusrtuserialmaster.cpp \
    qmodbustcpclient.cpp \
    qmodbustcpserver.cpp \
    qmodbusrtuserialslave.cpp \
    qmodbuspdu.cpp \
    qmodbusreplyex.cpp

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS

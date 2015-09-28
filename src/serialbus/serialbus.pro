TARGET = QtSerialBus
MODULE = serialbus
QT += core-private network

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
    qmodbusserialmaster.h \
    qmodbustcpclient.h \
    qmodbustcpserver.h \
    qmodbusserialslave.h

PRIVATE_HEADERS += \
    qcanbusdevice_p.h \
    qmodbusserver_p.h \
    qmodbusclient_p.h \
    qmodbusdevice_p.h \
    qmodbusserialmaster_p.h \
    qmodbustcpclient_p.h \
    qmodbustcpserver_p.h \
    qmodbusserialslave_p.h

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
    qmodbusserialmaster.cpp \
    qmodbustcpclient.cpp \
    qmodbustcpserver.cpp \
    qmodbusserialslave.cpp

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS

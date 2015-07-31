TARGET = QtSerialBus
MODULE = serialbus
QT += core-private serialport

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
    qmodbusslave.h \
    qmodbusfactory.h \

PRIVATE_HEADERS += \
    qcanbusdevice_p.h \
    qmodbusslave_p.h \

SOURCES += \
    qcanbusdevice.cpp \
    qcanbus.cpp \
    qcanbusfactory.cpp \
    qcanbusframe.cpp \
    qmodbus.cpp \
    qmodbusslave.cpp

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS \

TARGET = QtSerialBus
MODULE = serialbus
QT = core-private

QMAKE_DOCS = $$PWD/doc/qtserialbus.qdocconf

MODULE_PLUGIN_TYPES = \
    serialbuses

load(qt_module)

PUBLIC_HEADERS += \
    qcanbusdevice.h \
    qcanframe.h \
    qcanbus.h \
    qserialbusbackend.h \
    qserialbusglobal.h \
    qserialbusplugininterface.h \

PRIVATE_HEADERS += \
    qbusdummydevice.h \
    qcanbusdevice_p.h \
    qbusdummydevice_p.h \

SOURCES += \
    qbusdummydevice.cpp \
    qcanbusdevice.cpp \
    qcanframe.cpp \
    qserialbusbackend.cpp \
    qserialbusplugininterface.cpp \
    qcanbus.cpp

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS

TARGET = QtSerialBus
QT = core-private

QMAKE_DOCS = $$PWD/doc/qtserialbus.qdocconf

MODULE_PLUGIN_TYPES = \
    serialbuses

load(qt_module)

PUBLIC_HEADERS += \
    qcanbusdevice.h \
    qcanframe.h \
    qserialbus.h \
    qserialbusbackend.h \
    qserialbusdevice.h \
    qserialbusglobal.h \
    qserialbusplugininterface.h \

PRIVATE_HEADERS += \
    qbusdummydevice.h \
    qcanbusdevice_p.h

SOURCES += \
    qbusdummydevice.cpp \
    qcanbusdevice.cpp \
    qserialbus.cpp \
    qserialbusdevice.cpp \

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS

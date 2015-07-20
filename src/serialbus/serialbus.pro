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
    qserialbusglobal.h \
    qserialbusplugininterface.h \
    qcanbusfactory.h

PRIVATE_HEADERS += \
    qcanbusdevice_p.h

SOURCES += \
    qcanbusdevice.cpp \
    qcanframe.cpp \
    qserialbusplugininterface.cpp \
    qcanbus.cpp \
    qcanbusfactory.cpp

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS

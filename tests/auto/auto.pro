TEMPLATE = subdirs
SUBDIRS += cmake \
           qcanbusframe \
           qcanbusdevice \
           qmodbusdataunit \
           qmodbusreply \
           qmodbusdevice \
           qmodbuspdu \
           qmodbusclient \
           qmodbusserver \
           qmodbuscommevent \
           qmodbusadu \
           qmodbusdeviceidentification

QT_FOR_CONFIG += serialbus

qtConfig(modbus-serialport): SUBDIRS += qmodbusrtuserialmaster

!android: SUBDIRS += qcanbus

qcanbus.depends += plugins
qcanbusdevice.depends += plugins

SUBDIRS += plugins

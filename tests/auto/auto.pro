TEMPLATE = subdirs
SUBDIRS += cmake \
           qcanbusframe \
           qcanbus \
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

qcanbus.depends += plugins
qcanbusdevice.depends += plugins

SUBDIRS += plugins

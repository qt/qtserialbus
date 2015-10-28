TEMPLATE = subdirs
SUBDIRS += cmake \
           qcanbusframe \
           qcanbus \
           qcanbusdevice \
           qmodbusdataunit \
           qmodbusreply \
           qmodbusdevice \
           qmodbus \
           qmodbuspdu \
           qmodbusclient \
           qmodbusserver \

qcanbus.depends += plugins
qcanbusdevice.depends += plugins

SUBDIRS += plugins

TEMPLATE = subdirs
SUBDIRS += cmake \
           qcanbusframe \
           qcanbus \
           qcanbusdevice \
           qmodbusdataunit \
           qmodbusreply \
           qmodbusdevice \
           qmodbus \
           qmodbuspdu

qcanbus.depends += plugins
qcanbusdevice.depends += plugins

SUBDIRS += plugins

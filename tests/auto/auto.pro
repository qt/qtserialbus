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
           qmodbuscommevent

qcanbus.depends += plugins
qcanbusdevice.depends += plugins

SUBDIRS += plugins

TEMPLATE = subdirs
SUBDIRS += cmake \
           qcanbusframe \
           qcanbus \
           qcanbusdevice

qcanbus.depends += plugins
qcanbusdevice.depends += plugins

SUBDIRS += plugins



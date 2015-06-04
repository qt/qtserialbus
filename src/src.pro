TEMPLATE = subdirs

SUBDIRS += serialbus
serialbus.subdir = serialbus
serialbus.target = sub-serialbus

SUBDIRS += plugins
plugins.subdir = plugins
plugins.target = sub-plugins
plugins.depends = serialbus


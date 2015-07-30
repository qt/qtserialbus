TEMPLATE = subdirs

SUBDIRS += serialbus
serialbus.subdir = serialbus
serialbus.target = sub-serialbus

SUBDIRS += plugins
plugins.subdir = plugins
plugins.target = sub-plugins
plugins.depends = serialbus

SUBDIRS += tools
tools.subdir = tools
tools.target = sub-tools
tools.depends = serialbus

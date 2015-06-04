TEMPLATE = subdirs
SUBDIRS += generic
config_socketcan {
    SUBDIRS += can
}

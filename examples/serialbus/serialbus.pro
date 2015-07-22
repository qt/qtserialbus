TEMPLATE = subdirs

qtHaveModule(widgets) {
    SUBDIRS += can
}

qtHaveModule(widgets) {
    SUBDIRS += modbus
}

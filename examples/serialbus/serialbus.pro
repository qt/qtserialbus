TEMPLATE = subdirs

# right now compiles on Linux only
linux:qtHaveModule(widgets) {
    SUBDIRS += graphical
}

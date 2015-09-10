TEMPLATE = subdirs

SUBDIRS += thirdparty
thirdparty.subdir = 3rdparty
thirdparty.target = sub-3rdparty

SUBDIRS += serialbus
serialbus.subdir = serialbus
serialbus.target = sub-serialbus
serialbus.depends = thirdparty

SUBDIRS += plugins
plugins.subdir = plugins
plugins.target = sub-plugins
plugins.depends = serialbus

SUBDIRS += tools
tools.subdir = tools
tools.target = sub-tools
tools.depends = serialbus

!android:contains(QT_CONFIG, private_tests) {
    SUBDIRS += serialbus_doc_snippets
    serialbus_doc_snippets.subdir = serialbus/doc/snippets

    #plugin dependency required during static builds
    serialbus_doc_snippets.depends = serialbus plugins
}

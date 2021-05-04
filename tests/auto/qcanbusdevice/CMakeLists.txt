#####################################################################
## tst_qcanbusdevice Test:
#####################################################################

qt_internal_add_test(tst_qcanbusdevice
    SOURCES
        tst_qcanbusdevice.cpp
    PUBLIC_LIBRARIES
        Qt::SerialBus
)

# should be
# qt6_import_plugins(tst_qcanbusdevice INCLUDE GenericBusPlugin)
# but qt6_import_plugins does not work here
target_link_libraries(tst_qcanbusdevice PRIVATE GenericBusPlugin)

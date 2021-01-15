#####################################################################
## tst_qmodbusdevice Test:
#####################################################################

qt_internal_add_test(tst_qmodbusdevice
    SOURCES
        tst_qmodbusdevice.cpp
    PUBLIC_LIBRARIES
        Qt::SerialBus
)

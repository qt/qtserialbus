#####################################################################
## tst_qmodbusdeviceidentification Test:
#####################################################################

qt_internal_add_test(tst_qmodbusdeviceidentification
    SOURCES
        tst_qmodbusdeviceidentification.cpp
    PUBLIC_LIBRARIES
        Qt::Network
        Qt::SerialBus
)

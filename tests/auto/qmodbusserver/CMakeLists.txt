#####################################################################
## tst_qmodbusserver Test:
#####################################################################

qt_internal_add_test(tst_qmodbusserver
    SOURCES
        tst_qmodbusserver.cpp
    PUBLIC_LIBRARIES
        Qt::Network
        Qt::SerialBus
)

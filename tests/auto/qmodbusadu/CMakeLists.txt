#####################################################################
## tst_qmodbusadu Test:
#####################################################################

qt_internal_add_test(tst_qmodbusadu
    SOURCES
        tst_qmodbusadu.cpp
    PUBLIC_LIBRARIES
        Qt::Network
        Qt::SerialBus
        Qt::SerialBusPrivate
)

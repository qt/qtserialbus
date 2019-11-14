# Generated from qmodbusadu.pro.

#####################################################################
## tst_qmodbusadu Test:
#####################################################################

qt_add_test(tst_qmodbusadu
    SOURCES
        tst_qmodbusadu.cpp
    LIBRARIES
        Qt::SerialBusPrivate
    PUBLIC_LIBRARIES
        Qt::SerialBus
)

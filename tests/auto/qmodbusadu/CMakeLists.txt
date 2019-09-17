# Generated from qmodbusadu.pro.

#####################################################################
## tst_qmodbusadu Test:
#####################################################################

add_qt_test(tst_qmodbusadu
    SOURCES
        tst_qmodbusadu.cpp
    LIBRARIES
        Qt::SerialBusPrivate
    PUBLIC_LIBRARIES
        Qt::SerialBus
)

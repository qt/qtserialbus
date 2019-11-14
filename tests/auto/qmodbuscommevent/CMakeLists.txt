# Generated from qmodbuscommevent.pro.

#####################################################################
## tst_qmodbuscommevent Test:
#####################################################################

qt_add_test(tst_qmodbuscommevent
    SOURCES
        tst_qmodbuscommevent.cpp
    LIBRARIES
        Qt::SerialBusPrivate
    PUBLIC_LIBRARIES
        Qt::SerialBus
)

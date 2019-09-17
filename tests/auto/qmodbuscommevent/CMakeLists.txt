# Generated from qmodbuscommevent.pro.

#####################################################################
## tst_qmodbuscommevent Test:
#####################################################################

add_qt_test(tst_qmodbuscommevent
    SOURCES
        tst_qmodbuscommevent.cpp
    LIBRARIES
        Qt::SerialBusPrivate
    PUBLIC_LIBRARIES
        Qt::SerialBus
)

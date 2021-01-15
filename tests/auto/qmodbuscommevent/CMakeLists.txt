#####################################################################
## tst_qmodbuscommevent Test:
#####################################################################

qt_internal_add_test(tst_qmodbuscommevent
    SOURCES
        tst_qmodbuscommevent.cpp
    PUBLIC_LIBRARIES
        Qt::SerialBus
        Qt::SerialBusPrivate
)

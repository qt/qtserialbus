#####################################################################
## tst_qmodbuspdu Test:
#####################################################################

qt_internal_add_test(tst_qmodbuspdu
    SOURCES
        tst_qmodbuspdu.cpp
    PUBLIC_LIBRARIES
        Qt::Network
        Qt::SerialBus
)

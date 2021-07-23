#####################################################################
## tst_qmodbusreply Test:
#####################################################################

qt_internal_add_test(tst_qmodbusreply
    SOURCES
        tst_qmodbusreply.cpp
    PUBLIC_LIBRARIES
        Qt::Network
        Qt::SerialBus
)

#####################################################################
## tst_qcanbusframe Test:
#####################################################################

qt_internal_add_test(tst_qcanbusframe
    SOURCES
        tst_qcanbusframe.cpp
    PUBLIC_LIBRARIES
        Qt::Network
        Qt::SerialBus
)

qt_internal_add_test(tst_qmodbusrtuserialclient
    SOURCES
        tst_qmodbusrtuserialclient.cpp
    PUBLIC_LIBRARIES
        Qt::Network
        Qt::SerialBus
)

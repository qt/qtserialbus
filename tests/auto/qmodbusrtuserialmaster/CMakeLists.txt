#####################################################################
## tst_qmodbusrtuserialmaster Test:
#####################################################################

qt_internal_add_test(tst_qmodbusrtuserialmaster
    SOURCES
        tst_qmodbusrtuserialmaster.cpp
    PUBLIC_LIBRARIES
        Qt::SerialBus
)

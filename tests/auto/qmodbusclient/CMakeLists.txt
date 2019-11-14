# Generated from qmodbusclient.pro.

# special case begin
#if(NOT QT_CONFIG___contains___private_tests)
#    return()
#endif()
# special case end

#####################################################################
## tst_qmodbusclient Test:
#####################################################################

# special case begin
if (NOT QT_FEATURE_private_tests)
    return()
endif()
#special case end

qt_add_test(tst_qmodbusclient
    SOURCES
        tst_qmodbusclient.cpp
    LIBRARIES
        Qt::CorePrivate
        Qt::SerialBusPrivate
    PUBLIC_LIBRARIES
        Qt::SerialBus
)

#### Keys ignored in scope 1:.:.:qmodbusclient.pro:<TRUE>:
# _REQUIREMENTS = "contains(QT_CONFIG, private_tests)"

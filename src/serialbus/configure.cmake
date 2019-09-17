

#### Inputs



#### Libraries



#### Tests


if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../config.tests/socketcan/CMakeLists.txt")
    qt_config_compile_test("socketcan"
                           LABEL "Socket CAN"
                           PROJECT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../config.tests/socketcan")
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../config.tests/socketcan_fd/CMakeLists.txt")
    qt_config_compile_test("socketcan_fd"
                           LABEL "Socket CAN FD"
                           PROJECT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../config.tests/socketcan_fd")
endif()


#### Features

qt_feature("socketcan" PRIVATE
    LABEL "Socket CAN"
    CONDITION LINUX AND TEST_socketcan
)
qt_feature("socketcan_fd" PRIVATE
    LABEL "Socket CAN FD"
    CONDITION LINUX AND QT_FEATURE_socketcan AND TEST_socketcan_fd
)
qt_feature("modbus_serialport" PUBLIC
    LABEL "SerialPort Support"
    PURPOSE "Enables Serial-based Modbus Support"
    CONDITION TARGET Qt::SerialPort
)

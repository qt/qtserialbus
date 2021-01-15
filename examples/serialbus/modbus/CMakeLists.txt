add_subdirectory(master)
add_subdirectory(slave)
if(QT_FEATURE_modbus_serialport)
    add_subdirectory(adueditor)
endif()


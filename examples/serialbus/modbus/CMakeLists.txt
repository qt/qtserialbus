add_subdirectory(master)
add_subdirectory(slave)
add_subdirectory(custom)

if(QT_FEATURE_modbus_serialport)
    add_subdirectory(adueditor)
endif()


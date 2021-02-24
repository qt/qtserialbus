add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(custom)

if(QT_FEATURE_modbus_serialport)
    add_subdirectory(adueditor)
endif()


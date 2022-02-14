cmake_minimum_required(VERSION 3.16)
project(modbusserver LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/serialbus/modbus/server")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Network SerialBus Widgets)

qt_add_executable(modbusserver
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
    settingsdialog.cpp settingsdialog.h settingsdialog.ui
)

set_target_properties(modbusserver PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(modbusserver PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::SerialBus
    Qt::Widgets
)

# Resources:
set(server_resource_files
    "images/application-exit.png"
    "images/connect.png"
    "images/disconnect.png"
    "images/settings.png"
)

qt6_add_resources(modbusserver "server"
    PREFIX
        "/"
    FILES
        ${server_resource_files}
)

if(QT_FEATURE_modbus_serialport)
    target_link_libraries(modbusserver PUBLIC
        Qt::SerialPort
    )
endif()

install(TARGETS modbusserver
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

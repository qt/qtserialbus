cmake_minimum_required(VERSION 3.16)
project(customcommand LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/serialbus/modbus/custom")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Network SerialBus Widgets)

qt_add_executable(customcommand
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
    modbusclient.cpp modbusclient.h
    modbusserver.cpp modbusserver.h
    registermodel.cpp registermodel.h
)

set_target_properties(customcommand PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(customcommand PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::SerialBus
    Qt::Widgets
)

set(custom_resource_files
    "images/connect.png"
    "images/disconnect.png"
    "images/settings.png"
)

qt6_add_resources(customcommand "custom"
    PREFIX
        "/"
    FILES
        ${custom_resource_files}
)

install(TARGETS customcommand
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

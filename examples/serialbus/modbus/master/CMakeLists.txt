cmake_minimum_required(VERSION 3.14)
project(modbusmaster LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/serialbus/modbus/master")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS SerialBus)
find_package(Qt6 COMPONENTS Widgets)

qt_add_executable(modbusmaster
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
    settingsdialog.cpp settingsdialog.h settingsdialog.ui
    writeregistermodel.cpp writeregistermodel.h
)
set_target_properties(modbusmaster PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(modbusmaster PUBLIC
    Qt::Core
    Qt::Gui
    Qt::SerialBus
    Qt::Widgets
)


# Resources:
set(master_resource_files
    "images/application-exit.png"
    "images/connect.png"
    "images/disconnect.png"
    "images/settings.png"
)

qt6_add_resources(modbusmaster "master"
    PREFIX
        "/"
    FILES
        ${master_resource_files}
)

if(QT_FEATURE_modbus_serialport)
    target_link_libraries(modbusmaster PUBLIC
        Qt::SerialPort
    )
endif()

install(TARGETS modbusmaster
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

# Generated from slave.pro.

cmake_minimum_required(VERSION 3.14)
project(modbusslave LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(INSTALL_EXAMPLEDIR "examples/serialbus/modbus/slave")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS SerialBus)
find_package(Qt6 COMPONENTS Widgets)

# special case
get_target_property(public_features Qt6::SerialBus QT_ENABLED_PUBLIC_FEATURES)
if("modbus_serialport" IN_LIST public_features)
    find_package(Qt6 COMPONENTS SerialPort)
endif()

get_target_property(public_features Qt6::Widgets QT_ENABLED_PUBLIC_FEATURES)
if(NOT "combobox" IN_LIST public_features)
    return()
endif()
# special case

add_qt_gui_executable(modbusslave
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
    settingsdialog.cpp settingsdialog.h settingsdialog.ui
)
target_link_libraries(modbusslave PUBLIC
    Qt::Core
    Qt::Gui
    Qt::SerialBus
    Qt::Widgets
)


# Resources:
set(slave_resource_files
    "images/application-exit.png"
    "images/connect.png"
    "images/disconnect.png"
    "images/settings.png"
)

qt6_add_resources(modbusslave "slave"
    PREFIX
        "/"
    FILES
        ${slave_resource_files}
)

if(QT_FEATURE_modbus_serialport)
    target_link_libraries(modbusslave PUBLIC
        Qt::SerialPort
    )
endif()

install(TARGETS modbusslave
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

# Generated from adueditor.pro.

cmake_minimum_required(VERSION 3.14)
project(adueditor LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(INSTALL_EXAMPLEDIR "examples/serialbus/modbus/adueditor")

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

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS SerialBus)
find_package(Qt6 COMPONENTS SerialPort)
find_package(Qt6 COMPONENTS Widgets)

# special case begin
if (NOT QT_FEATURE_modbus_serialport)
    return()
endif()
# special case end

add_qt_gui_executable(adueditor
    interface.ui
    main.cpp
    mainwindow.cpp mainwindow.h
    modbustcpclient.cpp modbustcpclient.h modbustcpclient_p.h
    plaintextedit.h
)
target_include_directories(adueditor PUBLIC
    .
)

target_link_libraries(adueditor PRIVATE
    Qt::CorePrivate
    Qt::SerialBusPrivate
)

target_link_libraries(adueditor PUBLIC
    Qt::Core
    Qt::Gui
    Qt::SerialBus
    Qt::SerialPort
    Qt::Widgets
)

install(TARGETS adueditor
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

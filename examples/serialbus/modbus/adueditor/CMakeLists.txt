cmake_minimum_required(VERSION 3.14)
project(adueditor LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/serialbus/modbus/adueditor")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS SerialBus)
find_package(Qt6 COMPONENTS SerialPort)
find_package(Qt6 COMPONENTS Widgets)

qt_add_executable(adueditor
    interface.ui
    main.cpp
    mainwindow.cpp mainwindow.h
    modbustcpclient.cpp modbustcpclient.h modbustcpclient_p.h
    plaintextedit.h
)
set_target_properties(adueditor PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_include_directories(adueditor PUBLIC
    .
)

target_link_libraries(adueditor PUBLIC
    Qt::Core
    Qt::CorePrivate
    Qt::Gui
    Qt::SerialBus
    Qt::SerialBusPrivate
    Qt::SerialPort
    Qt::Widgets
)

install(TARGETS adueditor
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

cmake_minimum_required(VERSION 3.16)
project(can LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/serialbus/can")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Network)
find_package(Qt6 COMPONENTS SerialBus)
find_package(Qt6 COMPONENTS Widgets)

qt_add_executable(can
    bitratebox.cpp bitratebox.h
    canbusdeviceinfobox.h canbusdeviceinfobox.cpp canbusdeviceinfobox.ui
    canbusdeviceinfodialog.h canbusdeviceinfodialog.cpp canbusdeviceinfodialog.ui
    connectdialog.cpp connectdialog.h connectdialog.ui
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
    sendframebox.cpp sendframebox.h sendframebox.ui
    receivedframesmodel.cpp receivedframesmodel.h
    receivedframesview.cpp receivedframesview.h
)
set_target_properties(can PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(can PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::SerialBus
    Qt::Widgets
)


# Resources:
set(can_resource_files
    "images/application-exit.png"
    "images/clear.png"
    "images/connect.png"
    "images/disconnect.png"
)

qt6_add_resources(can "can"
    PREFIX
        "/"
    FILES
        ${can_resource_files}
)

install(TARGETS can
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

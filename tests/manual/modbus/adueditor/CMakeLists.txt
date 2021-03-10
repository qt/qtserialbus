qt_internal_add_manual_test(adueditor
        GUI
    SOURCES
        interface.ui
        main.cpp
        mainwindow.cpp mainwindow.h
        modbustcpclient.cpp modbustcpclient.h modbustcpclient_p.h
        plaintextedit.h
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::Gui
        Qt::SerialBus
        Qt::SerialBusPrivate
        Qt::SerialPort
        Qt::Widgets
    ENABLE_AUTOGEN_TOOLS
        uic
)

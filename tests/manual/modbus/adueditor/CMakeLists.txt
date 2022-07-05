# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
        Qt::Network
        Qt::SerialBus
        Qt::SerialBusPrivate
        Qt::SerialPort
        Qt::Widgets
    ENABLE_AUTOGEN_TOOLS
        uic
)

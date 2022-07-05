# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#####################################################################
## SystecCanBusPlugin Plugin:
#####################################################################

qt_internal_add_plugin(SystecCanBusPlugin
    OUTPUT_NAME qtsysteccanbus
    PLUGIN_TYPE canbus
    SOURCES
        main.cpp
        systeccan_symbols_p.h
        systeccanbackend.cpp systeccanbackend.h systeccanbackend_p.h
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::SerialBus
)

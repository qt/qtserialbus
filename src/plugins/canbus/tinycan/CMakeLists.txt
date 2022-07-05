# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#####################################################################
## TinyCanBusPlugin Plugin:
#####################################################################

qt_internal_add_plugin(TinyCanBusPlugin
    OUTPUT_NAME qttinycanbus
    PLUGIN_TYPE canbus
    SOURCES
        main.cpp
        tinycan_symbols_p.h
        tinycanbackend.cpp tinycanbackend.h tinycanbackend_p.h
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::SerialBus
)

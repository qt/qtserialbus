# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#####################################################################
## TestCanBusPlugin Plugin:
#####################################################################

qt_internal_add_plugin(TestCanBusPlugin
    OUTPUT_NAME qttestcanbus
    PLUGIN_TYPE canbus
    DEFAULT_IF FALSE
    STATIC
    SOURCES
        testcanbackend.cpp testcanbackend.h
        main.cpp
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Network
        Qt::SerialBus
)

# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#####################################################################
## VectorCanBusPlugin Plugin:
#####################################################################

qt_internal_add_plugin(VectorCanBusPlugin
    OUTPUT_NAME qtvectorcanbus
    PLUGIN_TYPE canbus
    SOURCES
        main.cpp
        vectorcan_symbols_p.h
        vectorcanbackend.cpp vectorcanbackend.h vectorcanbackend_p.h
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::SerialBus
)

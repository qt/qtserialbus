# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

if(TARGET Qt::Widgets AND TARGET Qt::SerialBus)
    add_subdirectory(modbus)
endif()

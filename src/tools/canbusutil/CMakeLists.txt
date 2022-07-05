# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#####################################################################
## canbusutil Tool:
#####################################################################

qt_internal_add_app(canbusutil
    TARGET_DESCRIPTION "Qt CAN Bus Util"
    SOURCES
        canbusutil.cpp canbusutil.h
        main.cpp
        readtask.cpp readtask.h
        sigtermhandler.cpp sigtermhandler.h
    LIBRARIES
        Qt::Network
        Qt::SerialBus
)
set_target_properties(canbusutil PROPERTIES WIN32_EXECUTABLE FALSE)

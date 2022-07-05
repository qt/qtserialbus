# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

if(NOT QT_FEATURE_private_tests)
    return()
endif()

#####################################################################
## tst_qmodbusclient Test:
#####################################################################

qt_internal_add_test(tst_qmodbusclient
    SOURCES
        tst_qmodbusclient.cpp
    PUBLIC_LIBRARIES
        Qt::CorePrivate
        Qt::Network
        Qt::SerialBus
        Qt::SerialBusPrivate
)


# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#####################################################################
## tst_qmodbusreply Test:
#####################################################################

qt_internal_add_test(tst_qmodbusreply
    SOURCES
        tst_qmodbusreply.cpp
    PUBLIC_LIBRARIES
        Qt::Network
        Qt::SerialBus
)

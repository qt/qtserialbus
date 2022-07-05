# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#####################################################################
## tst_qmodbusdataunit Test:
#####################################################################

qt_internal_add_test(tst_qmodbusdataunit
    SOURCES
        tst_qmodbusdataunit.cpp
    PUBLIC_LIBRARIES
        Qt::Network
        Qt::SerialBus
)

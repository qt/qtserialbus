# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#####################################################################
## tst_qcanbusdevice Test:
#####################################################################

qt_internal_add_test(tst_qcanbusdevice
    SOURCES
        tst_qcanbusdevice.cpp
    PUBLIC_LIBRARIES
        Qt::Network
        Qt::SerialBus
)

# should be
# qt6_import_plugins(tst_qcanbusdevice INCLUDE TestCanBusPlugin)
# but qt6_import_plugins does not work here
target_link_libraries(tst_qcanbusdevice PRIVATE TestCanBusPlugin)

# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_qmodbusdeviceidentification LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

#####################################################################
## tst_qmodbusdeviceidentification Test:
#####################################################################

qt_internal_add_test(tst_qmodbusdeviceidentification
    SOURCES
        tst_qmodbusdeviceidentification.cpp
    LIBRARIES
        Qt::Network
        Qt::SerialBus
)

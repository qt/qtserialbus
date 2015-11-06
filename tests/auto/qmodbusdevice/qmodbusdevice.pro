QT = core testlib serialbus
QT +=  core-private serialbus-private # to test QModbusDevicePrivate
TARGET = tst_qmodbusdevice
CONFIG += testcase no_private_qt_headers_warning c++11

SOURCES += tst_qmodbusdevice.cpp

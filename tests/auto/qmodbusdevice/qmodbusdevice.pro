QT = core testlib serialbus
QT +=  core-private serialbus-private # to test QModbusDevicePrivate
TARGET = tst_qmodbusdevice
CONFIG += testcase no_private_qt_headers_warning

SOURCES += tst_qmodbusdevice.cpp

TEMPLATE = lib
TARGET = modbus

CONFIG += staticlib
win32:include(libmodbus_win.pri)

load(qt_helper_lib)

HEADERS += modbus.h \
           modbus-private.h \
           modbus-rtu.h \
           modbus-rtu-private.h \
           modbus-tcp.h \
           modbus-tcp-private.h \
           modbusversion.h

SOURCES += modbus.c \
           modbus-data.c \
           modbus-rtu.c \
           modbus-tcp.c

config_libmodbus_accept4: DEFINES+=HAVE_ACCEPT4
config_libmodbus_byteswap: DEFINES+=HAVE_BYTESWAP_H
config_libmodbus_rs485: DEFINES+=HAVE_DECL_TIOCSRS485
config_libmodbus_strlcpy: DEFINES+=HAVE_STRLCPY
config_libmodbus_tiocmrts: DEFINES+=HAVE_DECL_TIOCM_RTS

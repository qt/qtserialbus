requires(!wince)
requires(!winrt) # enforced by qtserialport dependency
requires(!ios)   # enforced by qtserialport dependency
requires(!win32-msvc2012) # does not support variadic templates
requires(!win32-msvc2010) # not C++11 capabable -> will be removed from CI soon
requires(!win32-msvc2008) # not C++11 capabable -> will be removed from CI soon

lessThan(QT_MAJOR_VERSION, 5) {
    message("Cannot build current QtSerialBus sources with Qt version $${QT_VERSION}.")
}

load(configure)
qtCompileTest(socketcan)
qtCompileTest(socketcan_fd)

# Check we have a platform libmodbus in case we prefer the platform version.
qtCompileTest(libmodbus_systemlib)

qtCompileTest(libmodbus_accept4)
qtCompileTest(libmodbus_byteswap)
qtCompileTest(libmodbus_rs485)
qtCompileTest(libmodbus_strlcpy)
qtCompileTest(libmodbus_tiocmrts)
load(qt_parts)

linux {
    !config_socketcan:warning("You need linux/can.h and linux/can/raw.h linux headers for socketCAN support, disabling it")
    !config_socketcan_fd:warning("Newer kernel needed for flexible data-rate frame support 'canfd_frame'")
}

config_libmodbus_systemlib {
    message("Using platform's libmodbus")
} else {
    message("Using internal libmodbus")
}

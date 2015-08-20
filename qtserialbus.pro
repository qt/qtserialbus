requires(!wince)
requires(!winrt) # enforced by qtserialport dependency
requires(!ios)   # enforced by qtserialport dependency

lessThan(QT_MAJOR_VERSION, 5) {
    message("Cannot build current QtSerialBus sources with Qt version $${QT_VERSION}.")
}

load(configure)
qtCompileTest(socketcan)
qtCompileTest(socketcan_fd)
qtCompileTest(libmodbus)
load(qt_parts)

!config_libmodbus:warning("Cannot build libmodbus plugin. No libmodbus found")

linux {
    !config_socketcan:warning("You need linux/can.h and linux/can/raw.h linux headers for socketCAN support, disabling it")
    !config_socketcan_fd:warning("Newer kernel needed for flexible data-rate frame support 'canfd_frame'")
}

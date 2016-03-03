requires(!wince)
requires(!winrt) # enforced by qtserialport dependency
requires(!ios)   # enforced by qtserialport dependency
requires(!win32-msvc2012) # does not support variadic templates
requires(!win32-msvc2010) # not C++11 capabable -> will be removed from CI soon
requires(!win32-msvc2008) # not C++11 capabable -> will be removed from CI soon
requires(c++11)

lessThan(QT_MAJOR_VERSION, 5) {
    message("Cannot build current QtSerialBus sources with Qt version $${QT_VERSION}.")
}

load(configure)
qtCompileTest(socketcan)
qtCompileTest(socketcan_fd)
load(qt_parts)

linux {
    !config_socketcan:warning("You need linux/can.h and linux/can/raw.h linux headers for socketCAN support, disabling it")
    !config_socketcan_fd:warning("Newer kernel needed for flexible data-rate frame support 'canfd_frame'")
}

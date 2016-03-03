requires(!wince)
requires(!winrt) # enforced by qtserialport dependency
requires(!ios)   # enforced by qtserialport dependency
requires(!win32-msvc2012) # does not support variadic templates
requires(!win32-msvc2010) # not C++11 capabable -> will be removed from CI soon
requires(!win32-msvc2008) # not C++11 capabable -> will be removed from CI soon
requires(c++11)

defineTest(isGCCVersionSupported) {
    # The below will work for gcc 4.7 and up and also match gcc 5
    greaterThan(QT_GCC_MINOR_VERSION, 6):return(true)
    greaterThan(QT_GCC_MAJOR_VERSION, 4):return(true)
    warning("Using gcc version "$$QT_GCC_MAJOR_VERSION"."$$QT_GCC_MINOR_VERSION", but at least gcc version 4.7 is required")
    return(false)
}

gcc:!clang:!isGCCVersionSupported(): requires(false)

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

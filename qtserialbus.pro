requires(qtHaveModule(serialport))

lessThan(QT_MAJOR_VERSION, 5) {
    message("Cannot build current QtSerialBus sources with Qt version $${QT_VERSION}.")
}

load(configure)
qtCompileTest(language)
qtCompileTest(socketcan)
qtCompileTest(socketcan_fd)
load(qt_parts)

requires(config_language) # enforce defined set of C++11
!config_language:warning("Cannot compile the module with your compiler, missing C++11 language features.")

linux {
    !config_socketcan:warning("You need linux/can.h and linux/can/raw.h linux headers for socketCAN support, disabling it")
    !config_socketcan_fd:warning("Newer kernel needed for flexible data-rate frame support 'canfd_frame'")
}

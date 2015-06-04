lessThan(QT_MAJOR_VERSION, 5) {
    message("Cannot build current QtSerialBus sources with Qt version $${QT_VERSION}.")
}

load(configure)
!qtCompileTest(socketcan) {
    warning("You need linux/can.h and linux/can/raw.h linux headers for socketCAN support, disabling it")
}
!qtCompileTest(socketcan_fd) {
    warning("Newer kernel needed for flexible data-rate frame support 'canfd_frame'")
}


load(qt_parts)

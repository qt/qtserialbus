#####################################################################
## canbusutil Tool:
#####################################################################

qt_internal_add_app(canbusutil
    TARGET_DESCRIPTION "Qt CAN Bus Util"
    SOURCES
        canbusutil.cpp canbusutil.h
        main.cpp
        readtask.cpp readtask.h
        sigtermhandler.cpp sigtermhandler.h
    LIBRARIES
        Qt::Network
        Qt::SerialBus
)
set_target_properties(canbusutil PROPERTIES WIN32_EXECUTABLE FALSE)

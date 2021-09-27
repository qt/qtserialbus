#####################################################################
## canbusutil Tool:
#####################################################################

qt_get_tool_target_name(target_name canbusutil)
qt_internal_add_tool(${target_name}
    TARGET_DESCRIPTION "Qt CAN Bus Util"
    TOOLS_TARGET SerialBus
    SOURCES
        canbusutil.cpp canbusutil.h
        main.cpp
        readtask.cpp readtask.h
        sigtermhandler.cpp sigtermhandler.h
    LIBRARIES
        Qt::Network
        Qt::SerialBus
)
qt_internal_return_unless_building_tools()

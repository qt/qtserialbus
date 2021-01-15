qt_examples_build_begin()

if(NOT TARGET Qt::Widgets)
    return()
endif()
if(NOT TARGET Qt::SerialBus)
    return()
endif()
add_subdirectory(serialbus)

qt_examples_build_end()

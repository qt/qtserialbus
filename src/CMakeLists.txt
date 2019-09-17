# Generated from src.pro.

add_subdirectory(serialbus)
add_subdirectory(plugins)
add_subdirectory(tools)

# special case
if (NOT TARGET ANDROID AND QT_FEATURE_private_tests)
    add_subdirectory(serialbus/doc/snippets)
endif()
# special case

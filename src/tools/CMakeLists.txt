if(android_app OR (QT_FEATURE_commandlineparser AND NOT ANDROID))
    add_subdirectory(canbusutil)
endif()

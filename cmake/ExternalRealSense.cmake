# Download and set up libfreenect2

include(cmake/Externals.cmake)

ExternalProject_Add(realsense
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/realsense
        BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/realsense
        GIT_REPOSITORY "https://github.com/IntelRealSense/librealsense.git"
        GIT_TAG "v2.16.5"
        CMAKE_ARGS
        -DBUILD_EXAMPLES:BOOL=OFF
        -DBUILD_GRAPHICAL_EXAMPLES:BOOL=OFF
        -DBUILD_EASYLOGGINGPP:BOOL=OFF
        CMAKE_CACHE_ARGS
        -DCMAKE_BUILD_TYPE:STRING=Release
        -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF
        -DCMAKE_INSTALL_MESSAGE:BOOL=LAZY
        -DCMAKE_INSTALL_PREFIX:STRING=${FAST_EXTERNAL_INSTALL_DIR}
        )

list(APPEND LIBRARIES ${CMAKE_SHARED_LIBRARY_PREFIX}realsense2${CMAKE_SHARED_LIBRARY_SUFFIX})
list(APPEND FAST_EXTERNAL_DEPENDENCIES realsense)

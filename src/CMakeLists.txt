# Taken from https://gitlab.com/CLIUtils/modern-cmake/-/blob/master/examples/extended-project/src/CMakeLists.txt
# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

# Optionally glob, but only for CMake 3.12 or later:
# file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${ModernCMakeExample_SOURCE_DIR}/include/modern/*.hpp")
set(HEADER_LIST 
    "${chump_SOURCE_DIR}/include/exec.h"
    "${chump_SOURCE_DIR}/include/fetch.h"
    "${chump_SOURCE_DIR}/include/manager.h"
    "${chump_SOURCE_DIR}/include/package_list.h"
    "${chump_SOURCE_DIR}/include/package.h"
    "${chump_SOURCE_DIR}/include/uninstaller.h"
    "${chump_SOURCE_DIR}/include/util.h"
)

# Make an automatic library - will be static or dynamic based on user setting
add_library(chump_lib 
    STATIC
    fetch.cpp
    manager.cpp
    package_list.cpp
    package.cpp
    uninstaller.cpp
    util.cpp 
    ${HEADER_LIST}
)

# We need this directory, and users of our library will need it too
target_include_directories(chump_lib PUBLIC ../include)

# This depends on (header only) boost
target_link_libraries(chump_lib PRIVATE nlohmann_json::nlohmann_json libcurl PDcurses)

# All users of this library will need at least C++17
target_compile_features(chump_lib PUBLIC cxx_std_17)

# IDEs should put the headers in a nice place
source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${HEADER_LIST}
)

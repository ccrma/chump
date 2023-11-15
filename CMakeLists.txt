cmake_minimum_required(VERSION 3.26) # arbitrary cmake version

project(chump)

# Include(FetchContent)
enable_testing()


# Setup build

set(CMAKE_CXX_STANDARD 17)

set(SOURCES
  src/manager.h
  src/manager.cpp
  src/package.h
  src/exec.h
  src/fetch.h
  src/fetch.cpp
)

add_executable(chump src/main.cpp ${SOURCES})

# Fetch and link nlohmann json library
include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)

target_link_libraries(chump PRIVATE nlohmann_json::nlohmann_json)

##################
# Testing
##################

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0 # or a later release
)

FetchContent_MakeAvailable(Catch2)

add_executable(tests
    ${SOURCES}
    src/exec_test.cpp
    src/fetch_test.cpp
)

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(tests)

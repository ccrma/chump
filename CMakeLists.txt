cmake_minimum_required(VERSION 3.26) # arbitrary cmake version

project(chump)

# Include(FetchContent)
enable_testing()


# Setup build

set(CMAKE_CXX_STANDARD 17)

# Add src directory so I don't need to do #include "../exec.h" in the
# test directory
include_directories(src)

set(SOURCES
  src/manager.h
  src/manager.cpp
  src/package.h
  src/package.cpp
  src/exec.h
  src/fetch.h
  src/fetch.cpp
)

add_executable(chump src/main.cpp ${SOURCES})

##################
# Fetch and link nlohmann json library
##################
include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)

target_link_libraries(chump PRIVATE nlohmann_json::nlohmann_json)

##################
# Fetch and link Catch2 testing Library
##################
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0 # or a later release
)

FetchContent_MakeAvailable(Catch2)

##################
# Testing
##################

add_executable(tests
    ${SOURCES}
    src/test/exec_test.cpp
    src/test/fetch_test.cpp
)

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain nlohmann_json::nlohmann_json)

# Configure the location of the test data directory
set(TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/test/data)

# Copy test data files to the build directory
configure_file(${TEST_DATA_DIR}/test-data.json ${CMAKE_CURRENT_BINARY_DIR}/data/test-data.json COPYONLY)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(tests)

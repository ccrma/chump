##################
# Fetch and link Catch2 testing Library
##################
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0 # or a later release
  # CMAKE_ARGS     -DBUILD_SHARED_LIBS=OFF
)

FetchContent_MakeAvailable(Catch2)

add_executable(testlib
    exec_test.cpp
    fetch_test.cpp
    package_test.cpp
    package_list_test.cpp   
)

target_compile_features(testlib PRIVATE cxx_std_17)

# Should be linked to the main library, as well as the Catch2 testing library
target_link_libraries(testlib PRIVATE chump_lib Catch2::Catch2WithMain nlohmann_json::nlohmann_json libcurl)


# Define and copy test data files to the build directory
set(TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/data)

configure_file(
    ${TEST_DATA_DIR}/test-data.json
    ${CMAKE_CURRENT_BINARY_DIR}/data/test-data.json COPYONLY
)

configure_file(
     ${TEST_DATA_DIR}/test-package-list.json
    ${CMAKE_CURRENT_BINARY_DIR}/data/test-package-list.json COPYONLY
)



# If you register a test, then ctest and make test will run it.
# You can also run examples and check the output, as well.

include(CTest)
include(Catch)
catch_discover_tests(testlib)

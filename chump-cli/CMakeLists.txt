add_executable(chump main.cpp)
target_compile_features(chump PRIVATE cxx_std_17)

target_link_libraries(chump PRIVATE chump_lib nlohmann_json::nlohmann_json libcurl)

if (WIN32)
   target_link_libraries(chump PRIVATE PDcurses)
ELSE()
   target_link_libraries(chump PRIVATE ${CURSES_LIBRARIES})
ENDIF()

cmake_minimum_required(VERSION 3.22) # arbitrary cmake version

project(
  chump
  VERSION 0.1
  DESCRIPTION "The CHUck Manager of Packages"
  LANGUAGES CXX
)

# Setup build
set(CMAKE_CXX_STANDARD 17)

# Let's nicely support folders in IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

include(CTest)


##################
# Fetch and link nlohmann json library
##################
include(FetchContent)

FetchContent_Declare(
  json 
  URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)
FetchContent_MakeAvailable(json)


##################
# Add libcurl
##################
#set(USE_WINDOWSSSPI  ON) 
#set(USE_SCHANNEL ON) 

FetchContent_Declare(curl
  URL https://github.com/curl/curl/releases/download/curl-8_1_2/curl-8.1.2.tar.xz
  CMAKE_ARGS -DBUILD_SHARED_LIBS=OFF -DBUILD_STATIC_LIBS=ON -DCURL_STATICLIB
)

FetchContent_MakeAvailable(curl)

##################
# Add curses
##################
IF (WIN32)
  add_subdirectory(extern/pdcurses)
ELSE()
  find_package(Curses REQUIRED)
  include_directories(${CURSES_INCLUDE_DIR})
  # target_link_libraries(chump PRIVATE ${CURSES_LIBRARIES})
  # target_include_directories(chump PRIVATE ${CURSES_INCLUDE_DIRS})
ENDIF()



# All subdirectories
add_subdirectory(src)
# add_subdirectory(extern/pdcurses)
add_subdirectory(chump-cli)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

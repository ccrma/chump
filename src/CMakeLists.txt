cmake_minimum_required(VERSION 3.26) # arbitrary cmake version

project(chump)

set(CMAKE_CXX_STANDARD 17)

add_executable(chump main.cpp
  manager.h
  package.h
)

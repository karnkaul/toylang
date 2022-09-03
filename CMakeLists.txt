cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(toylang)

option(TL_BUILD_TESTS "Build toylang tests" ON)

add_subdirectory(toylang)

if(TL_BUILD_TESTS)
  add_subdirectory(tests)
endif()

cmake_minimum_required(VERSION 3.10)
project(json)
enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")

# option(ENABLE_ASSERTIONS "Enable/disable assertions" OFF)

# if(NOT ENABLE_ASSERTIONS)
# add_compile_definitions(NDEBUG)
# endif()
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(FindGTest)
include_directories(${GTEST_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src/include)

add_subdirectory(src)
add_subdirectory(test)

# add_subdirectory(example)
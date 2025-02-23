cmake_minimum_required(VERSION 3.10)

# Set the project name
project(Simulator)

# Set C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -pedantic")

# Add the source files for the library
file(GLOB SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../common/src/*.cpp"
)

add_library(Simulator STATIC ${SOURCES})

target_include_directories(Simulator
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(myrobot
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(myrobot
  PRIVATE
    Simulator
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../common/headers)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/headers)

target_link_options(myrobot PRIVATE -rdynamic)

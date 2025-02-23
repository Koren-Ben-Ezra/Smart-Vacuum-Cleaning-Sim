cmake_minimum_required(VERSION 3.10)

# Set the project name
project(MySharedLibrary)

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

# Add the include directories (if any)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../common/headers)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/headers)

# Create the shared library
add_library(_AlgoA SHARED ${SOURCES})

# Set the output directory for the shared library
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../algorithms)

set_target_properties(_AlgoA PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../algorithms
)

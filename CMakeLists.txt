cmake_minimum_required (VERSION 3.10)

project(hostcontrold)

# add cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# prepare use of pkgconfig
find_package(PkgConfig REQUIRED)

# predefine compile flags
set(cxxflags -Werror -Wall -Wextra -Wconversion -Wpedantic -Wno-unused-function)

# use C++17 standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

include_directories(SYSTEM libs/json/src)

add_subdirectory(hostcontrold)

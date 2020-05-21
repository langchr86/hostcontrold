cmake_minimum_required(VERSION 3.10)

project(hostcontrold)

# add cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# prepare use of pkgconfig
find_package(PkgConfig REQUIRED)

# predefine compile flags
set(cxxflags -Werror -Wall -Wextra -Wconversion -Wpedantic -Wno-unused-command-line-argument)

# use C++14 standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)

include_directories(SYSTEM libs/json/src)

enable_testing()

add_subdirectory(hostcontrold)

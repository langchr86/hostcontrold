cmake_minimum_required (VERSION 3.5)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(cpplint)

# project settings
PROJECT(hostcontrold)
set(HC_APP "hostcontrold")

# find external dependencies
find_package(PkgConfig REQUIRED)
pkg_search_module(libsystemd REQUIRED IMPORTED_TARGET libsystemd)
pkg_search_module(liboping REQUIRED IMPORTED_TARGET liboping)

# collect sources
file(GLOB SRC_MAIN "src/main.cpp")
file(GLOB_RECURSE SRC_CHECKED "src/*.c*" "src/*.h*")
list(REMOVE_ITEM SRC_CHECKED ${SRC_MAIN})

# prepare application target
add_executable(${HC_APP}
  ${SRC_CHECKED}
  ${SRC_MAIN}
)

# code checks
setup_cpplint(${HC_APP} ${SRC_MAIN} ${SRC_CHECKED})

# include directories
target_include_directories(${HC_APP} PRIVATE
  "libs/json/src"
  "src"
)

# set compiler flags
set(HC_CFLAGS "-g -Wall -Werror -Wno-unused-function -Wno-unused-parameter -Wno-reorder")
set_target_properties(${HC_APP} PROPERTIES COMPILE_FLAGS "${HC_CFLAGS}")
set_target_properties(${HC_APP} PROPERTIES CXX_STANDARD 14)

# link to dependencies
target_link_libraries(${HC_APP}
  PkgConfig::libsystemd
  PkgConfig::liboping
)

# install steps
INSTALL(TARGETS ${HC_APP}
  RUNTIME DESTINATION /usr/sbin/
)
INSTALL(FILES hostcontrold.service
  DESTINATION /etc/systemd/system/
)


cmake_minimum_required (VERSION 3.5)

# project settings
PROJECT(hostcontrold)
set(HC_APP "hostcontrold")

# dependencies
include(ExternalProject)
ExternalProject_Add(liboping
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/liboping
  CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/libs/liboping/autogen.sh && ${CMAKE_CURRENT_SOURCE_DIR}/libs/liboping/configure --prefix=${CMAKE_CURRENT_SOURCE_DIR}/libs/liboping --without-perl-bindings
  PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/libs/liboping
  BUILD_COMMAND ${MAKE}
  BUILD_IN_SOURCE 1
)

# collect sources
file(GLOB SRC_MAIN "src/main.cpp")
file(GLOB_RECURSE SRC_CHECKED "src/*.c*" "src/*.h*")
list(REMOVE_ITEM SRC_CHECKED ${SRC_MAIN})

# prepare application target
add_executable(${HC_APP}
  ${SRC_CHECKED}
  ${SRC_MAIN}
)

# include directories
target_include_directories(${HC_APP} PRIVATE
  "libs/liboping/src"
  "src"
)

# set compiler flags
set(HC_CFLAGS "-g -Wall -Werror -Wno-unused-function -Wno-unused-parameter -Wno-reorder")
set_target_properties(${HC_APP} PROPERTIES COMPILE_FLAGS "${HC_CFLAGS}")
set_target_properties(${HC_APP} PROPERTIES CXX_STANDARD 14)

# link to dependencies
add_dependencies(${HC_APP} liboping)
target_link_libraries(${HC_APP}
  "${CMAKE_CURRENT_SOURCE_DIR}/libs/liboping/lib/liboping.a"
)

# install steps
INSTALL(TARGETS ${HC_APP}
  RUNTIME DESTINATION /usr/sbin/
)
INSTALL(FILES hostcontrold.service
  DESTINATION /etc/systemd/system/
)


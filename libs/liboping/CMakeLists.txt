cmake_minimum_required(VERSION 3.5)

include(ExternalProject)
externalproject_add(liboping
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
    CONFIGURE_COMMAND
      ${CMAKE_CURRENT_SOURCE_DIR}/autogen.sh
      && ${CMAKE_CURRENT_SOURCE_DIR}/configure --prefix=${CMAKE_CURRENT_SOURCE_DIR} --without-perl-bindings
    PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/libs/liboping
    BUILD_COMMAND ${MAKE}
    BUILD_IN_SOURCE 1
)

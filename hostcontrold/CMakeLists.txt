include(cpplint)

project(hostcontrold)

# find external dependencies
pkg_search_module(libsystemd REQUIRED IMPORTED_TARGET libsystemd)
pkg_search_module(liboping REQUIRED IMPORTED_TARGET liboping oping)

# collect sources
file(GLOB_RECURSE src_lib src/*.cpp)
file(GLOB_RECURSE hdr_lib src/*.h*)
file(GLOB_RECURSE src_main src/main.cpp)
list(REMOVE_ITEM src_lib ${src_main})

# internal library target
add_library(${PROJECT_NAME}_lib STATIC ${src_lib})
target_include_directories(${PROJECT_NAME}_lib PUBLIC src)
target_link_libraries(${PROJECT_NAME}_lib
    PRIVATE
      PkgConfig::libsystemd
      PkgConfig::liboping
)

# main app target
add_executable(${PROJECT_NAME} ${src_main})
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_lib)

# style checks
setup_cpplint(${PROJECT_NAME}_lib ${src_lib} ${hdr_lib})
setup_cpplint(${PROJECT_NAME} ${src_main})

# install steps
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION /usr/sbin/
)
install(FILES hostcontrold.service
    DESTINATION /etc/systemd/system/
)
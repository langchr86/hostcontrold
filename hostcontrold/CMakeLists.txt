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

# prepare application target
add_executable(${PROJECT_NAME} ${src_main} ${src_lib})
setup_cpplint(${PROJECT_NAME} ${src_main} ${src_lib})

# link to dependencies
target_link_libraries(${PROJECT_NAME}
    PRIVATE
      PkgConfig::libsystemd
      PkgConfig::liboping
)
target_include_directories(${PROJECT_NAME} PRIVATE src)

# install steps
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION /usr/sbin/
)
install(FILES hostcontrold.service
    DESTINATION /etc/systemd/system/
)

cmake_minimum_required(VERSION 3.10)

include(cpplint)

# find external dependencies
pkg_search_module(gtest REQUIRED IMPORTED_TARGET gtest)
pkg_search_module(gtest_main REQUIRED IMPORTED_TARGET gtest_main)
pkg_search_module(gmock REQUIRED IMPORTED_TARGET gmock)

# collect sources
file(GLOB_RECURSE src_test *.cpp)
file(GLOB_RECURSE hdr_test *.h*)

# unittest target
add_executable(${PROJECT_NAME}_test ${src_test})
add_test(${PROJECT_NAME} ${PROJECT_NAME}_test)
target_include_directories(${PROJECT_NAME}_test PRIVATE ./)
target_link_libraries(${PROJECT_NAME}_test
    PRIVATE
      ${PROJECT_NAME}_lib
      PkgConfig::gtest
      PkgConfig::gtest_main
      PkgConfig::gmock
)

# style checks
setup_cpplint(${PROJECT_NAME}_test ${src_test} ${hdr_test})

# set compiler flags
target_compile_options(${PROJECT_NAME}_test PRIVATE ${cxxflags})

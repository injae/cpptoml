cmake_minimum_required(VERSION 3.12)

set(CPPM_VERSION 0.0.12)
include(cmake/cppm_loader.cmake)
cppm_project()

project(tomlpp VERSION 0.6.0 LANGUAGES C CXX)
cppm_setting()
cppm_cxx_standard(17)
cppm_compiler_option(DEFAULT)


if(CMAKE_BUILD_TYPE STREQUAL "Debug")
find_cppkg(Catch2 2.9.1  MODULE Catch2::Catch2 TYPE lib)
endif()


cppm_target_define(tomlpp INTERFACE)

cppm_unit_test_area()
if(TOMLPP_BUILD_TESTING)

cppm_target_define(parse_test BINARY
SOURCES
    test/test.cpp
)

endif()
end_cppm_unit_test_area()


set(global_deps )
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
   list(APPEND global_deps PUBLIC Catch2)
endif()
cppm_target_dependencies(tomlpp
   ${global_deps})

cppm_target_dependencies(parse_test
   ${global_deps}
   tomlpp)


cppm_target_install(tomlpp)


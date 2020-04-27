cmake_minimum_required(VERSION 3.12)

if(NOT IS_CPPM_LOADED)
    include(cmake/cppm_tool.cmake)
endif()
cppm_project()

project(tomlpp VERSION 0.6.0 LANGUAGES C CXX)
cppm_setting()
cppm_cxx_standard(17)
cppm_compiler_option(DEFAULT)



cppm_target_define(tomlpp INTERFACE)


set(global_deps )
cppm_target_dependencies(tomlpp
   ${global_deps})


cppm_target_install(tomlpp)


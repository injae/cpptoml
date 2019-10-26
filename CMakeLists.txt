cmake_minimum_required(VERSION 3.12)

include(cmake/cppm_tool.cmake)
cppm_project()
project(cpptoml VERSION 0.5.0 LANGUAGES C CXX)
cppm_setting()

cppm_cxx_standard(17)
cppm_compiler_option(DEFAULT)



cppm_target_define(cpptoml INTERFACE )


cppm_target_dependencies(cpptoml 
)

cppm_target_install(cpptoml)


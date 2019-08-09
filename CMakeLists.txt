cmake_minimum_required(VERSION 2.8)
project(fluent-bit-pgsql)

# Macro to build source code
macro(FLB_PLUGIN name src deps)
  add_library(flb-${name} SHARED ${src})
  set_target_properties(flb-${name} PROPERTIES PREFIX "")
  set_target_properties(flb-${name} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
  target_link_libraries(flb-${name} ${deps})
  target_link_libraries(flb-${name} pq)
endmacro()

# Fluent Bit source code environment
if(NOT DEFINED FLB_SOURCE OR FLB_SOURCE STREQUAL "")
  message(FATAL_ERROR "No Fluent Bit source directory has been defined")
endif()

# Build Options
option(FLB_DEBUG              "Build with debug symbols"      No)

# Validate plugin name
if(NOT DEFINED PLUGIN_NAME OR PLUGIN_NAME STREQUAL "")
  message(FATAL_ERROR "Plugin name is not defined")
endif()

# Info
message(STATUS "Fluent Bit source code: " ${FLB_SOURCE})

# Include Headers
set(FLB_PATH_ROOT_SOURCE ${FLB_SOURCE})
include(${FLB_SOURCE}/cmake/libraries.cmake)
include(${FLB_SOURCE}/cmake/headers.cmake)

# Enable Debug
if(FLB_DEBUG)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

# Build plugin
add_subdirectory(${PLUGIN_NAME})

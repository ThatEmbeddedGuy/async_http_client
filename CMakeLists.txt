#######################################
# Asynchronous http client build system
#######################################

cmake_minimum_required(VERSION 2.6)

project(async_http_client)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
  message( FATAL_ERROR "You can't build in source directory")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -Wall -O2")
message( STATUS "CMAKE_CXX_FLAGS  ${CMAKE_CXX_FLAGS}" )

find_package(Boost 1.65 COMPONENTS 
  program_options
  filesystem
  thread
  system
  chrono 
  REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

aux_source_directory(${CMAKE_SOURCE_DIR} SOURCES)
message(STATUS "sources: ${SOURCES}")

add_executable(main "${SOURCES}")

#target_link_libraries(main ${async_http_client} LINK_PUBLIC ${Boost_LIBRARIES})
target_link_libraries(main LINK_PUBLIC ${Boost_LIBRARIES} crypto ssl)

cmake_minimum_required(VERSION 3.11)
project(parselib)

add_subdirectory(src)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")

option(PARSELIB_ENABLE_TESTS OFF)
if (${PARSELIB_ENABLE_TESTS})
    add_subdirectory(googletest)
    include_directories(
        "${CMAKE_CURRENT_SOURCE_DIR}/googletest/googletest/include"
    )
    include_directories(
        "${CMAKE_CURRENT_SOURCE_DIR}/googletest/googlemock/include"
    )
    enable_testing()

    add_subdirectory(tests)
endif()

option(PARSELIB_ENABLE_SAMPLES OFF)
if (${PARSELIB_ENABLE_SAMPLES})
    add_subdirectory(samples)
endif()

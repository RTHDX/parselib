cmake_minimum_required(VERSION 3.11)
project(parselib_tests)

create_test_executable(
    TARGET ariphmetic_test
    SOURCES ariphmetic.cpp
    LIBS parselib
)

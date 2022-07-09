cmake_minimum_required(VERSION 3.11)
project(parselib)

add_library(${PROJECT_NAME}
    language.hpp parsers.hpp lexer.hpp exceptions.hpp constants.hpp
    language.cpp parsers.cpp lexer.cpp
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)

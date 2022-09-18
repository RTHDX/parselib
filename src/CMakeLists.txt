cmake_minimum_required(VERSION 3.11)
project(parselib)

create_library(
    TARGET ${PROJECT_NAME}
    SOURCES language.cpp parsers.cpp lexer.cpp
    HEADERS language.hpp parsers.hpp lexer.hpp exceptions.hpp constants.hpp
)

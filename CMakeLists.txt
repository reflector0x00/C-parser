cmake_minimum_required(VERSION 2.6)
project(lalr_parser)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall")
set(SOURCES_LALR sources/lalr_main.cpp)
set(SOURCES_C_PARSER sources/c_parser_main.cpp sources/c_parser.cpp)

include_directories("include")

add_executable(lalr ${SOURCES_LALR})
add_executable(c_parser ${SOURCES_C_PARSER})
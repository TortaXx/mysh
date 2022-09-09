cmake_minimum_required(VERSION 3.0)
project(mysh C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(mysh
        main.c
        main.h)

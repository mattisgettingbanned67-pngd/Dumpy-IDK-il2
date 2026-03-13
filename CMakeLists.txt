cmake_minimum_required(VERSION 3.16)
project(IL2CppOffsetDumper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(IL2CppOffsetDumper
    src/main.cpp
)

if (MSVC)
    target_compile_options(IL2CppOffsetDumper PRIVATE /W4 /permissive-)
else()
    target_compile_options(IL2CppOffsetDumper PRIVATE -Wall -Wextra -Wpedantic)
endif()

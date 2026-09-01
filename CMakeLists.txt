cmake_minimum_required(VERSION 3.20)

project(
    FinCon
    VERSION 1.0.0
    DESCRIPTION "Financial exception investigation and control system"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(fincon
    src/main.cpp
)

target_include_directories(fincon
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(fincon PRIVATE
        /W4
        /permissive-
    )
else()
    target_compile_options(fincon PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion
    )
endif()
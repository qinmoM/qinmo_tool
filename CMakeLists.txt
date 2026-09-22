cmake_minimum_required(VERSION 3.10)
project(qinmo_tool)


add_library(qinmo_tool
    ./src/temp.cpp
)


target_compile_features(qinmo_tool
    PUBLIC
        cxx_std_11
)

target_include_directories(qinmo_tool
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
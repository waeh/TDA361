cmake_minimum_required ( VERSION 3.0.2 )

project ( computer-graphics-labs )

set ( CMAKE_EXPORT_COMPILE_COMMANDS ON )
set ( CMAKE_CXX_FLAGS "-std=c++11" )

add_subdirectory ( labhelper )
add_subdirectory ( lab1 )
add_subdirectory ( lab2-textures )
add_subdirectory ( lab3-camera )
add_subdirectory ( lab4-cubemapping )

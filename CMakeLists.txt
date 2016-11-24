cmake_minimum_required ( VERSION 3.0.2 )

project ( computer-graphics-labs )

set ( CMAKE_EXPORT_COMPILE_COMMANDS ON )
set ( CMAKE_BUILD_TYPE DEBUG )

add_subdirectory ( labhelper )
add_subdirectory ( lab1 )
add_subdirectory ( lab2-textures )
add_subdirectory ( lab3-camera )
add_subdirectory ( lab4-cubemapping )
add_subdirectory ( lab5-rendertotexture )
add_subdirectory ( lab6-shadowmaps )
add_subdirectory ( pathtracer )
add_subdirectory ( project )

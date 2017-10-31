cmake_minimum_required ( VERSION 3.0.2 )

project ( lab2 )

# Build and link executable.
add_executable ( lab2 lab2_main.cpp )
target_link_libraries ( lab2 labhelper )
config_build_output()

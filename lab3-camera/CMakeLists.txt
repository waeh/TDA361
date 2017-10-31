cmake_minimum_required ( VERSION 3.0.2 )

project ( lab3 )

# Build and link executable.
add_executable ( lab3 lab3_main.cpp )
target_link_libraries ( lab3 labhelper )
config_build_output()

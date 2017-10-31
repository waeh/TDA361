cmake_minimum_required ( VERSION 3.0.2 )

project ( lab4 )

# Build and link executable.
add_executable ( lab4 lab4_main.cpp )
target_link_libraries ( lab4 labhelper )
config_build_output()

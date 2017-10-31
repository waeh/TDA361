cmake_minimum_required ( VERSION 3.0.2 )

project ( lab5 )

# Build and link executable.
add_executable ( lab5 lab5_main.cpp hdr.cpp )
target_link_libraries ( lab5 labhelper )
config_build_output()

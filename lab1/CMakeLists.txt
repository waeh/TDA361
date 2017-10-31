cmake_minimum_required ( VERSION 3.0.2 )

project ( lab1 )


# Build and link executable.
add_executable ( lab1 lab1_main.cpp )
target_link_libraries ( lab1 labhelper )
config_build_output()

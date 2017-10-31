cmake_minimum_required ( VERSION 3.0.2 )

project ( lab6 )

# Build and link executable.
add_executable ( lab6 lab6_main.cpp hdr.cpp fbo.cpp )
target_link_libraries ( lab6 labhelper )
config_build_output()

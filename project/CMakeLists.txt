cmake_minimum_required ( VERSION 3.0.2 )

project ( project )

# Build and link executable.
add_executable ( project
    main.cpp
    fbo.cpp
    hdr.cpp
    heightfield.cpp
    ParticleSystem.cpp
    )

target_link_libraries ( project labhelper )
config_build_output()

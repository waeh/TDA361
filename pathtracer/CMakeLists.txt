cmake_minimum_required ( VERSION 3.0.2 )

project ( pathtracer )

set ( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )

include_directories ( ${CMAKE_SOURCE_DIR}/labhelper )

# External libraries.
include_directories ( ${CMAKE_SOURCE_DIR}/external/imgui-1.49 )
include_directories ( ${CMAKE_SOURCE_DIR}/external/stb-1.0 )

find_package ( SDL2 REQUIRED )
include_directories ( ${SDL2_INCLUDE_DIRS} )

find_package ( embree 2.12 REQUIRED )
include_directories ( ${EMBREE_INCLUDE_PATH} )

find_package ( OpenMP REQUIRED )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

# Build and link executable.
add_executable ( pathtracer
    main.cpp
    Pathtracer.cpp
    sampling.cpp
    HDRImage.cpp
    embree.cpp
    material.cpp
    )

target_link_libraries ( pathtracer labhelper ${EMBREE_LIBRARY} )

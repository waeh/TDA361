cmake_minimum_required ( VERSION 3.0.2 )

project ( labhelper )

# External libraries.
include_directories ( ${CMAKE_SOURCE_DIR}/external/stb-1.0 )
include_directories ( ${CMAKE_SOURCE_DIR}/external/tinyobjloader-1.0.0 )

include_directories ( ${CMAKE_SOURCE_DIR}/external/imgui-1.49 )
add_subdirectory ( ${CMAKE_SOURCE_DIR}/external/imgui-1.49 ${CMAKE_CURRENT_BINARY_DIR}/imgui )

find_package ( SDL2 REQUIRED )
include_directories ( ${SDL2_INCLUDE_DIRS} )
if ( POLICY CMP0004 )
    cmake_policy( SET CMP0004 OLD )
endif()

find_package ( glm REQUIRED )
include_directories ( ${GLM_INCLUDE_DIRS} )
find_package ( GLEW REQUIRED )
include_directories ( ${GLEW_INCLUDE_DIRS} )
find_package ( OpenGL REQUIRED )
include_directories ( ${OPENGL_INCLUDE_DIR} )


# Build and link library.
add_library ( labhelper 
    labhelper.cpp 
    Model.cpp
    imgui_impl_sdl_gl3.cpp
    )

target_link_libraries ( labhelper
    imgui
    ${SDL2_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${OPENGL_LIBRARY}
    )

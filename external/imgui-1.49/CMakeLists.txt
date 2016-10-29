cmake_minimum_required ( VERSION 3.0.2 )

project ( imgui )

# Build and link executable.
add_library ( imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui_draw.cpp 
    ) 

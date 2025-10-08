cmake_minimum_required(VERSION 4.0)
project(imgui)

# Create imgui library with all source files
add_library(${PROJECT_NAME} STATIC
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    imgui_impl_sdl3.cpp
    imgui_impl_sdlrenderer3.cpp
)

# Create alias target for namespaced usage
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# Set include directories
target_include_directories(${PROJECT_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link SDL3 (required for imgui_impl_sdl3.cpp)
find_library(SDL3
    NAMES libSDL3.dll.a SDL3
    PATHS ${CMAKE_SOURCE_DIR}/Dependencies/sdl3/lib
    NO_DEFAULT_PATH
)

if(SDL3)
    target_link_libraries(${PROJECT_NAME} PUBLIC ${SDL3})
else()
    message(WARNING "SDL3 library not found for imgui")
endif()

# Include SDL3 headers (needed for the implementation files)
target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/Dependencies/sdl3/include
)

cmake_minimum_required(VERSION 4.0)
project(CPURenderer)

set(CMAKE_CXX_STANDARD 20)

#set(RAYLIB_VERSION 5.5)
#find_package(raylib ${RAYLIB_VERSION} QUIET) # QUIET or REQUIRED
#if (NOT raylib_FOUND) # If there's none, fetch and build raylib
#    include(FetchContent)
#    FetchContent_Declare(
#            raylib
#            DOWNLOAD_EXTRACT_TIMESTAMP OFF
#            URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
#    )
#    FetchContent_GetProperties(raylib)
#    if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
#        set(FETCHCONTENT_QUIET NO)
#        FetchContent_MakeAvailable(raylib)
#    endif()
#endif()


#include_directories(Dependencies/raylib/include)

include_directories(Dependencies/sdl3/include)
include_directories(Dependencies/glm)

add_executable(${PROJECT_NAME})
add_subdirectory(src)

find_library(SDL3
        NAMES libSDL3.dll.a SDL3
        PATHS Dependencies/sdl3/lib
        NO_DEFAULT_PATH
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL3})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/Dependencies/sdl3/bin/SDL3.dll
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Dependencies/glm
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/Dependencies/glm
)

# Cross platform solution
#target_link_libraries(${PROJECT_NAME} raylib)

# Manual configure local solution
#find_library(RAYLIB
#        NAMES libraylib.a raylib
#        PATHS Dependencies/raylib/lib
#        NO_DEFAULT_PATH
#)
#
#target_link_libraries(${PROJECT_NAME} ${RAYLIB})
#
#if(WIN32)
#    target_link_libraries(${PROJECT_NAME}
#            winmm
#            gdi32
#            opengl32
#            user32
#            kernel32
#    )
#
#    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#            COMMAND ${CMAKE_COMMAND} -E copy_if_different
#            ${CMAKE_SOURCE_DIR}/Dependencies/raylib/lib/raylib.dll
#            $<TARGET_FILE_DIR:${PROJECT_NAME}>
#    )
#
#    # Copy resources folder to build directory
#    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#            COMMAND ${CMAKE_COMMAND} -E copy_directory
#            ${CMAKE_SOURCE_DIR}/resources
#            $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
#    )
#endif()

cmake_minimum_required(VERSION 4.0)
project(CPURenderer)

set(CMAKE_CXX_STANDARD 20)

include_directories(Dependencies/sdl3/include)
include_directories(Dependencies/glm)
include_directories(Dependencies/assimp/include)

add_executable(${PROJECT_NAME})
add_subdirectory(src)

find_library(SDL3
        NAMES libSDL3.dll.a SDL3
        PATHS Dependencies/sdl3/lib
        NO_DEFAULT_PATH
)
find_library(ASSIMP
        NAMES assimp-vc143-mt.lib
        PATHS Dependencies/assimp/lib/x64
        NO_DEFAULT_PATH
)
link_libraries(Dependencies/assimp/lib/x64/)

target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL3})
target_link_libraries(${PROJECT_NAME} PRIVATE ${ASSIMP})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/Dependencies/sdl3/bin/SDL3.dll
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/Dependencies/assimp/bin/assimp-vc143-mt.dll
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Dependencies/glm
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/Dependencies/glm
)
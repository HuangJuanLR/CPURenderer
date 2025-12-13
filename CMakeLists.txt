cmake_minimum_required(VERSION 4.0)
project(CPURenderer)

set(CMAKE_CXX_STANDARD 20)
if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")
endif()

include_directories(Dependencies/sdl3/include)
include_directories(Dependencies/glm)
include_directories(Dependencies/pfr)
include_directories(Dependencies/assimp/include)
include_directories(Dependencies/plog/include)
include_directories(Dependencies/imgui/include)
include_directories(Dependencies/entt/include)

add_executable(${PROJECT_NAME})
add_subdirectory(src)
add_subdirectory(Dependencies/plog)
add_subdirectory(Dependencies/imgui)

find_library(SDL3
        NAMES libSDL3.dll.a SDL3
        PATHS Dependencies/sdl3/lib
        NO_DEFAULT_PATH
)
find_library(ZLIB
        NAMES libz.dll.a zlib
        PATHS Dependencies/zlib/lib
        NO_DEFAULT_PATH
)
find_library(ASSIMP
        NAMES libassimp.dll.a ASSIMP
        PATHS Dependencies/assimp/lib
        NO_DEFAULT_PATH
)

target_link_libraries(${PROJECT_NAME} ${SDL3})
target_link_libraries(${PROJECT_NAME} ${ZLIB})
target_link_libraries(${PROJECT_NAME} ${ASSIMP})
target_link_libraries(${PROJECT_NAME} plog::plog)
target_link_libraries(${PROJECT_NAME} imgui)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/Dependencies/sdl3/bin/SDL3.dll
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/Dependencies/zlib/bin/zlib1.dll
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/Dependencies/assimp/bin/libassimp-6.dll
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Dependencies/glm
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/Dependencies/glm
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Dependencies/pfr
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/Dependencies/pfr
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
)

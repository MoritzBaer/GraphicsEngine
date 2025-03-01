cmake_minimum_required(VERSION 3.28)

project(GRAPHICS_ENGINE)

option(BUILD_WITH_PROFILING "Build for profiling" ON)
option(BUILD_DEMO_APPS "Build demo apps" ON)

set(CMAKE_CXX_STANDARD 23)

# set(CMAKE_VERBOSE_MAKEFILE ON)

# Allow for file name macro set relative to source folder
string(LENGTH "${CMAKE_SOURCE_DIR}/src/" SOURCE_PATH_SIZE)
add_compile_definitions(SOURCE_PATH_SIZE=${SOURCE_PATH_SIZE})

message("User components source path: ${USER_COMPONENTS_SOURCE_PATH}")

if(DEFINED USER_COMPONENTS_SOURCE_PATH)
    message("USER_COMPONENTS_SOURCE_PATH defined, adding definition")
    add_compile_definitions(USER_COMPONENTS_SOURCE=${USER_COMPONENTS_SOURCE_PATH})
endif()

if(BUILD_WITH_PROFILING)
    add_definitions("-DRUN_PROFILER")
    message("-- BUILDING WITH PROFILER ACTIVE")
else()
    message("-- building without profiling")
endif()

find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)

message("Vulkan found: ${Vulkan_FOUND} (with include dirs ${Vulkan_INCLUDE_DIR} and libraries ${Vulkan_LIBRARIES})")

include_directories("dependencies/glfw-3.3.9.bin.WIN64/include")
include_directories("dependencies/VMA")
include_directories("dependencies/GLM/")
include_directories(${Vulkan_INCLUDE_DIR})

include_directories("src/external/json-parsing")
include_directories("src/external/stb")
include_directories("src/Engine")

file(GLOB_RECURSE EngineFiles
    if(DEFINED USER_COMPONENTS_SOURCE_PATH) USER_COMPONENTS_SOURCE_PATH endif()
    "src/Engine/*.h"
    "src/Engine/*.cpp"  
)

aux_source_directory(src/external EXTERNAL)

add_library(Engine STATIC ${EngineFiles})

macro(make_app NAME ENTRY_POINT)
    add_executable(${NAME} "src/${ENTRY_POINT}.cpp")

    target_link_libraries(${NAME} Engine)

    target_link_libraries(${NAME} "${CMAKE_SOURCE_DIR}/dependencies/glfw-3.3.9.bin.WIN64/lib-vc2022/glfw3_mt.lib")
    target_link_libraries(${NAME} Vulkan::Vulkan)
    target_link_libraries(${NAME} Vulkan::shaderc_combined)

    # To store profiles
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Release/profiles ${CMAKE_BINARY_DIR}/Debug/profiles)

    # To keep assets in the same place as the binaries
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        add_custom_command(TARGET ${NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res ${CMAKE_BINARY_DIR}/Debug/res)
        add_custom_command(TARGET ${NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/debug-symbols ${CMAKE_BINARY_DIR}/Debug)
    else()
        add_custom_command(TARGET ${NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res ${CMAKE_BINARY_DIR}/Release/res)
        add_custom_command(TARGET ${NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/res//VkLayer_khronos_validation.pdb ${CMAKE_BINARY_DIR}/Release)
    endif()
endmacro()

if(${BUILD_DEMO_APPS})
    make_app(TestApp test)
    make_app(DebugApp main)
endif()
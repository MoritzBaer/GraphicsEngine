cmake_minimum_required(VERSION 3.28)

project(GRAPHICS_ENGINE)

option(BUILD_WITH_PROFILING "Build for profiling" ON)
option(BUILD_DEMO_APPS "Build demo apps" ON)
option(DELIVER_RESOURCES "Copy resources to binary directory" ON)

set(CMAKE_CXX_STANDARD 23)

# set(CMAKE_VERBOSE_MAKEFILE ON)

# Allow for file name macro set relative to source folder
string(LENGTH "${PROJECT_SOURCE_DIR}/src/" SOURCE_PATH_SIZE)
add_compile_definitions(SOURCE_PATH_SIZE=${SOURCE_PATH_SIZE})

message("-- User components source path: ${USER_COMPONENTS_SOURCE_PATH}")

if(DEFINED USER_COMPONENTS_SOURCE_PATH)
    message("-- USER_COMPONENTS_SOURCE_PATH defined, adding definition")
    add_compile_definitions(USER_COMPONENTS_SOURCE=${USER_COMPONENTS_SOURCE_PATH})
endif()

if(DEFINED USER_SCRIPTS_SOURCE_PATH)
    message("-- USER_SCRIPTS_SOURCE_PATH defined, adding definition")
    add_compile_definitions(USER_SCRIPTS_SOURCE=${USER_SCRIPTS_SOURCE_PATH})
else()
    if(BUILD_DEMO_APPS)
        add_compile_definitions(USER_SCRIPTS_SOURCE="${PROJECT_SOURCE_DIR}/src/Demo/Scripts.h")
    endif()
endif()

if(BUILD_WITH_PROFILING)
    add_definitions("-DRUN_PROFILER")
    message("-- BUILDING WITH PROFILER ACTIVE")
else()
    message("-- building without profiling")
endif()

find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)

file(GLOB_RECURSE EngineFiles
    if(DEFINED USER_COMPONENTS_SOURCE_PATH) USER_COMPONENTS_SOURCE_PATH endif()
    if(DEFINED USER_SCRIPTS_SOURCE_PATH) USER_SCRIPTS_SOURCE_PATH endif()
    "src/Engine/*.h"
    "src/Engine/*.cpp"
)

aux_source_directory(src/external EXTERNAL)

add_compile_options(/Zc:preprocessor)
add_library(Engine STATIC ${EngineFiles})

target_include_directories(Engine PUBLIC "dependencies/glfw-3.3.9.bin.WIN64/include")
target_include_directories(Engine PUBLIC "dependencies/VMA")
target_include_directories(Engine PUBLIC "dependencies/GLM/")
target_include_directories(Engine PUBLIC ${Vulkan_INCLUDE_DIR})

target_include_directories(Engine PUBLIC "src/external/json-parsing")
target_include_directories(Engine PUBLIC "src/external/stb")
target_include_directories(Engine PUBLIC "src/Engine")

target_link_libraries(Engine PUBLIC "${PROJECT_SOURCE_DIR}/dependencies/glfw-3.3.9.bin.WIN64/lib-vc2022/glfw3_mt.lib")
target_link_libraries(Engine PUBLIC Vulkan::Vulkan)
target_link_libraries(Engine PUBLIC Vulkan::shaderc_combined)

# To keep assets in the same place as the binaries
if(DELIVER_RESOURCES)
    message("-- Delivering engine resources to binary directory")

    if(CMAKE_BUILD_TYPE MATCHES Debug)
        add_custom_command(TARGET Engine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/res ${CMAKE_BINARY_DIR}/Debug/res)
        add_custom_command(TARGET Engine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/debug-symbols ${CMAKE_BINARY_DIR}/Debug)
    else()
        add_custom_command(TARGET Engine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/res ${CMAKE_BINARY_DIR}/Release/res)
    endif()
endif()

macro(make_app NAME ENTRY_POINT)
    message("-- Building demo app ${NAME}")
    add_executable(${NAME} "src/${ENTRY_POINT}.cpp")

    target_include_directories(${NAME} PRIVATE "src/Demo")
    target_link_libraries(${NAME} Engine)

    # To store profiles
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Release/profiles ${CMAKE_BINARY_DIR}/Debug/profiles)

    message("-- Build type for GameEngine: ${CMAKE_BUILD_TYPE}")
endmacro()

if(${BUILD_DEMO_APPS})
    make_app(TestApp test)
    make_app(DebugApp main)
endif()
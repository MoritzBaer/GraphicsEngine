cmake_minimum_required(VERSION 4.2.1)

project(GRAPHICS_ENGINE)

option(BUILD_WITH_PROFILING "Build for profiling" ON)
option(BUILD_DEMO_APPS "Build demo apps" ON)
option(DELIVER_RESOURCES "Copy resources to binary directory" ON)

set(CMAKE_CXX_STANDARD 23)

# set(CMAKE_VERBOSE_MAKEFILE ON)
file(GLOB_RECURSE EngineFiles
    if(DEFINED USER_COMPONENTS_SOURCE_PATH) USER_COMPONENTS_SOURCE_PATH endif()
    if(DEFINED USER_SCRIPTS_SOURCE_PATH) USER_SCRIPTS_SOURCE_PATH endif()
    "src/Engine/*.h"
    "src/Engine/*.cpp"
)

add_library(Engine STATIC ${EngineFiles})

# Allow for file name macro set relative to source folder
string(LENGTH "${PROJECT_SOURCE_DIR}/src/" SOURCE_PATH_SIZE)
target_compile_definitions(Engine PUBLIC SOURCE_PATH_SIZE=${SOURCE_PATH_SIZE})

message(STATUS "User components source path: ${USER_COMPONENTS_SOURCE_PATH}")

if(DEFINED USER_COMPONENTS_SOURCE_PATH)
    message(STATUS "USER_COMPONENTS_SOURCE_PATH defined, adding definition")
    target_compile_definitions(Engine PUBLIC USER_COMPONENTS_SOURCE=${USER_COMPONENTS_SOURCE_PATH})
endif()

if(DEFINED USER_SCRIPTS_SOURCE_PATH)
    message(STATUS "USER_SCRIPTS_SOURCE_PATH defined, adding definition")
    add_compile_definitions(USER_SCRIPTS_SOURCE=${USER_SCRIPTS_SOURCE_PATH})
else()
    if(BUILD_DEMO_APPS)
        add_compile_definitions(USER_SCRIPTS_SOURCE="${PROJECT_SOURCE_DIR}/src/Demo/Scripts.h")
    endif()
endif()

if(BUILD_WITH_PROFILING)
    target_compile_definitions(Engine PUBLIC "-DRUN_PROFILER")
    message(STATUS "BUILDING WITH PROFILER ACTIVE")
else()
    message(STATUS "building without profiling")
endif()

add_compile_definitions(CMAKE_EXPORT_COMPILE_COMMANDS=1)

find_package(Vulkan REQUIRED COMPONENTS shaderc_combined glslang SPIRV-Tools)
find_package(glfw3 REQUIRED)

message("shaderc path: ${Vulkan_shaderc_combined_LIBRARY}")

aux_source_directory(src/external EXTERNAL)

# Uncomment if building with msvc
#target_compile_options(Engine PUBLIC /Zc:preprocessor)

target_include_directories(Engine PUBLIC "dependencies/glfw-3.3.9.bin.WIN64/include")
target_include_directories(Engine PUBLIC "dependencies/VMA")
target_include_directories(Engine PUBLIC "dependencies/GLM/")
target_include_directories(Engine PUBLIC ${Vulkan_INCLUDE_DIR})
target_include_directories(Engine PUBLIC ${GLFW3_INCLUDE_DIR})

target_include_directories(Engine PUBLIC "src/external/json-parsing")
target_include_directories(Engine PUBLIC "src/external/stb")
target_include_directories(Engine PUBLIC "src/Engine")

# Uncomment if compiling for windows
#target_link_libraries(Engine PUBLIC "${PROJECT_SOURCE_DIR}/dependencies/glfw-3.3.9.bin.WIN64/lib-vc2022/glfw3_mt.lib")
target_link_libraries(Engine PUBLIC Vulkan::Vulkan)
target_link_libraries(Engine PUBLIC Vulkan::shaderc_combined)
target_link_libraries(Engine PUBLIC Vulkan::glslang)
target_link_libraries(Engine PUBLIC Vulkan::SPIRV-Tools)
target_link_libraries(Engine PUBLIC SPIRV-Tools-opt)
target_link_libraries(Engine PUBLIC glfw)

# To keep assets in the same place as the binaries
# TODO: Figure out compiler-independent delivery
if(DELIVER_RESOURCES)
    message(STATUS "Delivering engine resources to binary directory")
    add_custom_command(TARGET Engine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/res ${CMAKE_BINARY_DIR}/res)
    add_custom_command(TARGET Engine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/debug-symbols ${CMAKE_BINARY_DIR})
endif()

macro(make_app NAME ENTRY_POINT)
    message(STATUS "Building demo app ${NAME}")
    add_executable(${NAME} "src/${ENTRY_POINT}.cpp")

    target_include_directories(${NAME} PRIVATE "src/Demo")
    target_include_directories(${NAME} PRIVATE "src/")
    target_link_libraries(${NAME} Engine)

    # To store profiles
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Release/profiles ${CMAKE_BINARY_DIR}/Debug/profiles)

    message(STATUS "Build type for GameEngine: ${CMAKE_BUILD_TYPE}")
endmacro()

if(${BUILD_DEMO_APPS})
    make_app(TestApp test)
    make_app(DebugApp main)

    add_executable(MinimalTestApp "src/test.cpp" "src/Engine/Core/ECS.cpp")
    target_include_directories(MinimalTestApp PRIVATE "src/")
    target_include_directories(MinimalTestApp PRIVATE "src/Engine")
    target_include_directories(MinimalTestApp PRIVATE "src/external/json-parsing")

    target_compile_definitions(MinimalTestApp PRIVATE SOURCE_PATH_SIZE=${SOURCE_PATH_SIZE})
endif()
cmake_minimum_required(VERSION 3.28)

project(GRAPHICS_ENGINE)

option(BUILD_WITH_PROFILING "Build for profiling" OFF) # Not working atm. Would be nicer, but here we are
set(BUILD_WITH_PROFILING TRUE)

set(CMAKE_CXX_STANDARD 23)

# Allow for file name macro set relative to source folder
string(LENGTH "${CMAKE_SOURCE_DIR}/src/" SOURCE_PATH_SIZE)
add_definitions("-DSOURCE_PATH_SIZE=${SOURCE_PATH_SIZE}")

if(BUILD_WITH_PROFILING)
    add_definitions("-DRUN_PROFILER")
    message("-- BUILDING WITH PROFILER ACTIVE")
else()
    message("-- building without profiling")
endif()

find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)

include_directories("dependencies/glfw-3.3.9.bin.WIN64/include")
include_directories("dependencies/VMA")
include_directories("dependencies/GLM/")
include_directories(${Vulkan_INCLUDE_DIR})

include_directories("src/external/imgui")
include_directories("src/Engine")
include_directories("src/Editor")

file(GLOB ImGUIFiles "src/external/imgui/*.h" "src/external/imgui/*.cpp")

file(GLOB_RECURSE SourceFiles
    "src/Engine/*.h"
    "src/Engine/*.cpp"
    "src/external/imgui/backends/imgui_impl_vulkan.cpp"
    "src/external/imgui/backends/imgui_impl_glfw.cpp"
    "src/external/imgui/imgui_draw.cpp"
    "src/external/imgui/imgui_tables.cpp"
    "src/external/imgui/imgui_widgets.cpp"
    "src/external/imgui/imgui.cpp"
)

aux_source_directory(src/external EXTERNAL)

macro(make_app NAME ENTRY_POINT)
    add_executable(${NAME} "src/${ENTRY_POINT}.cpp" ${SourceFiles})

    target_link_libraries(${NAME} "${CMAKE_SOURCE_DIR}/dependencies/glfw-3.3.9.bin.WIN64/lib-vc2022/glfw3_mt.lib")
    target_link_libraries(${NAME} Vulkan::Vulkan)
    target_link_libraries(${NAME} Vulkan::shaderc_combined)

    # To store profiles
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Release/profiles ${CMAKE_BINARY_DIR}/Debug/profiles)

    # To keep assets in the same place as the binaries
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        add_custom_command(TARGET ${NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res ${CMAKE_BINARY_DIR}/Debug/res)
    else()
        add_custom_command(TARGET ${NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res ${CMAKE_BINARY_DIR}/Release/res)
    endif()
endmacro()

make_app(TestApp test)
make_app(DebugApp main)
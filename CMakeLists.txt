cmake_minimum_required(VERSION 3.24)

project(GRAPHICS_ENGINE)

set(CMAKE_CXX_STANDARD 23)

find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)

include_directories("dependencies/glfw-3.3.9.bin.WIN64/include/GLFW")
include_directories("dependencies/stb")
include_directories(${Vulkan_INCLUDE_DIR})

file(GLOB_RECURSE SourceFiles CONFIGURE_DEPENDS "src/Engine/*.h" "src/Engine/*.cpp" )

add_executable(MAIN "src/main.cpp" ${HeaderFiles} ${SourceFiles})

target_compile_options(MAIN PRIVATE "/MD$<$<CONFIG:Debug>:d>")

target_include_directories(MAIN PRIVATE ${CMAKE_SOURCE_DIR}/dependencies/GLM)

target_link_libraries(MAIN "${CMAKE_SOURCE_DIR}/dependencies/glfw-3.3.9.bin.WIN64/lib-vc2022/glfw3_mt.lib")
target_link_libraries(MAIN Vulkan::Vulkan)
target_link_libraries(MAIN Vulkan::shaderc_combined)
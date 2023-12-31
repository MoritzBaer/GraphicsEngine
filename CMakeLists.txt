cmake_minimum_required(VERSION 3.21)

project(GRAPHICS_ENGINE)

set(CMAKE_CXX_STANDARD 23)

find_package(Vulkan REQUIRED)

include_directories("dependencies/glfw-3.3.9.bin.WIN64/include/GLFW")
include_directories(${Vulkan_INCLUDE_DIR})

file(GLOB_RECURSE SourceFiles CONFIGURE_DEPENDS "src/Engine/*.h" "src/Engine/*.cpp" )

add_executable(MAIN "src/main.cpp" ${HeaderFiles} ${SourceFiles})

target_link_libraries(MAIN "${CMAKE_SOURCE_DIR}/dependencies/glfw-3.3.9.bin.WIN64/lib-vc2022/glfw3_mt.lib")
target_link_libraries(MAIN Vulkan::Vulkan)
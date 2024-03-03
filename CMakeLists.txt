cmake_minimum_required(VERSION 3.24)

project(GRAPHICS_ENGINE)

set(CMAKE_CXX_STANDARD 23)

find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)

include_directories("dependencies/glfw-3.3.9.bin.WIN64/include/GLFW")
include_directories("dependencies/VMA")
include_directories(${Vulkan_INCLUDE_DIR})

file(GLOB_RECURSE SourceFiles
    "src/Engine/*.h"    
    "src/Engine/*.cpp"    
)

add_executable(MAIN "src/main.cpp" ${SourceFiles}) 

target_link_libraries(MAIN "${CMAKE_SOURCE_DIR}/dependencies/glfw-3.3.9.bin.WIN64/lib-vc2022/glfw3_mt.lib")
target_link_libraries(MAIN Vulkan::Vulkan)
target_link_libraries(MAIN Vulkan::shaderc_combined)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_custom_command(TARGET MAIN POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res ${CMAKE_BINARY_DIR}/Debug/res)
else()
    add_custom_command(TARGET MAIN POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res ${CMAKE_BINARY_DIR}/Release/res)
endif()
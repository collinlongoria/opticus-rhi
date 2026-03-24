cmake_minimum_required(VERSION 3.16)
project(Opticus VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp", "src/*.hpp", "include/*.hpp")

add_library(Opticus STATIC ${SOURCES})

target_include_directories(Opticus PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_include_directories(Opticus PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(Opticus PRIVATE
        Vulkan::Vulkan
)

if(MSVC)
    target_compile_options(Opticus PRIVATE /W4 /WX)
else()
    target_compile_options(Opticus PRIVATE -Wall -Wextra -Werror)
endif()
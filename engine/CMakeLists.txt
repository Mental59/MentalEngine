cmake_minimum_required(VERSION 3.30)

project(MentalEngine LANGUAGES CXX VERSION 0.0.1)

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS include/*.hpp)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})

add_library(MentalEngine STATIC ${SOURCES} ${HEADERS})

find_package(Vulkan REQUIRED)

target_include_directories(MentalEngine PRIVATE include ${Vulkan_INCLUDE_DIRS})
target_link_libraries(MentalEngine PRIVATE Vulkan::Vulkan)
target_compile_definitions(MentalEngine PRIVATE VULKAN_HPP_NO_EXCEPTIONS MENTAL_WITH_VULKAN)

cmake_minimum_required(VERSION 3.30)

project(MentalEngine LANGUAGES CXX VERSION 0.0.1)

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS include/*.hpp)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})

add_library(MentalEngine STATIC ${SOURCES} ${HEADERS})

find_package(Vulkan REQUIRED)

if(WIN32)
target_compile_definitions(MentalEngine PRIVATE MENTAL_WIN32 VK_USE_PLATFORM_WIN32_KHR)
elseif(LINUX)
target_compile_definitions(MentalEngine PRIVATE MENTAL_LINUX VK_USE_PLATFORM_XCB_KHR)
else()
message(FATAL_ERROR "Unsupported platform")
endif()

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
set(GLFW_INSTALL OFF CACHE BOOL "")

add_subdirectory(deps/glfw)

target_include_directories(MentalEngine PUBLIC include ${Vulkan_INCLUDE_DIRS})
target_link_libraries(MentalEngine PRIVATE Vulkan::Vulkan glfw)
target_compile_definitions(MentalEngine PRIVATE VULKAN_HPP_NO_EXCEPTIONS MENTAL_WITH_VULKAN VK_NO_PROTOTYPES VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)

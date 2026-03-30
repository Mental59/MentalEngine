cmake_minimum_required(VERSION 3.30)

project(MentalEngine LANGUAGES CXX VERSION 0.0.1)

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS include/*.hpp)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})

add_library(MentalEngine STATIC ${SOURCES} ${HEADERS})

find_package(Vulkan REQUIRED)

set(MENTAL_VULKAN_SDK "$ENV{VULKAN_SDK}")
if(NOT MENTAL_VULKAN_SDK)
  message(FATAL_ERROR "VULKAN_SDK environment variable is not set")
endif()

set(MENTAL_GLM_INCLUDE_DIR "${MENTAL_VULKAN_SDK}/Include")
set(MENTAL_SLANG_INCLUDE_DIR "${MENTAL_VULKAN_SDK}/Include")
set(MENTAL_SLANG_IMPORT_LIB "${MENTAL_VULKAN_SDK}/Lib/slang.lib")
set(MENTAL_SLANG_DLL "${MENTAL_VULKAN_SDK}/Bin/slang.dll")
set(MENTAL_SLANG_RT_DLL "${MENTAL_VULKAN_SDK}/Bin/slang-rt.dll")

foreach(MENTAL_REQUIRED_PATH
    "${MENTAL_GLM_INCLUDE_DIR}/glm"
    "${MENTAL_SLANG_INCLUDE_DIR}/slang"
    "${MENTAL_SLANG_IMPORT_LIB}"
    "${MENTAL_SLANG_DLL}"
    "${MENTAL_SLANG_RT_DLL}")
  if(NOT EXISTS "${MENTAL_REQUIRED_PATH}")
    message(FATAL_ERROR "Required Vulkan SDK dependency path is missing: ${MENTAL_REQUIRED_PATH}")
  endif()
endforeach()

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

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE "${MENTAL_GLM_INCLUDE_DIR}")

add_library(slang SHARED IMPORTED GLOBAL)
set_target_properties(slang PROPERTIES
  IMPORTED_IMPLIB "${MENTAL_SLANG_IMPORT_LIB}"
  IMPORTED_LOCATION "${MENTAL_SLANG_DLL}"
  INTERFACE_INCLUDE_DIRECTORIES "${MENTAL_SLANG_INCLUDE_DIR}"
)

add_library(imgui STATIC
  deps/imgui/imgui.cpp
  deps/imgui/imgui_draw.cpp
  deps/imgui/imgui_tables.cpp
  deps/imgui/imgui_widgets.cpp
  deps/imgui/backends/imgui_impl_glfw.cpp
  deps/imgui/backends/imgui_impl_vulkan.cpp
)
target_include_directories(imgui PUBLIC deps/imgui deps/imgui/backends)
target_link_libraries(imgui PRIVATE glfw Vulkan::Vulkan)
target_compile_definitions(imgui PRIVATE IMGUI_IMPL_VULKAN_NO_PROTOTYPES NOMINMAX)

target_include_directories(MentalEngine PUBLIC include ${Vulkan_INCLUDE_DIRS})
target_link_libraries(MentalEngine PRIVATE Vulkan::Vulkan glfw glm slang imgui)
target_compile_definitions(MentalEngine PRIVATE MENTAL_WITH_VULKAN VK_NO_PROTOTYPES VMA_STATIC_VULKAN_FUNCTIONS=0 VMA_DYNAMIC_VULKAN_FUNCTIONS=0 NOMINMAX)

set(MENTAL_SLANG_DLL "${MENTAL_SLANG_DLL}" CACHE INTERNAL "Path to the Slang runtime DLL")
set(MENTAL_SLANG_RT_DLL "${MENTAL_SLANG_RT_DLL}" CACHE INTERNAL "Path to the Slang runtime support DLL")

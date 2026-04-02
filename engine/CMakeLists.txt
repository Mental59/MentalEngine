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
add_subdirectory(deps/entt)

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
target_link_libraries(MentalEngine
  PUBLIC glm EnTT::EnTT
  PRIVATE Vulkan::Vulkan glfw slang imgui
)
target_compile_definitions(MentalEngine PRIVATE MENTAL_WITH_VULKAN VK_NO_PROTOTYPES VMA_STATIC_VULKAN_FUNCTIONS=0 VMA_DYNAMIC_VULKAN_FUNCTIONS=0 NOMINMAX)

add_executable(MentalEngineSceneTests tests/editorSceneTests.cpp)
target_link_libraries(MentalEngineSceneTests PRIVATE MentalEngine)
add_test(NAME MentalEngineSceneTests COMMAND MentalEngineSceneTests)

add_executable(MentalEngineCameraTests tests/cameraTests.cpp src/camera/camera.cpp)
target_include_directories(MentalEngineCameraTests PRIVATE include)
target_link_libraries(MentalEngineCameraTests PRIVATE glm)
add_test(NAME MentalEngineCameraTests COMMAND MentalEngineCameraTests)

add_executable(MentalEngineSceneCameraTests tests/sceneCameraTests.cpp)
target_link_libraries(MentalEngineSceneCameraTests PRIVATE MentalEngine)
add_test(NAME MentalEngineSceneCameraTests COMMAND MentalEngineSceneCameraTests)

add_executable(MentalEngineEditorApplicationTests tests/editorApplicationTests.cpp)
target_link_libraries(MentalEngineEditorApplicationTests PRIVATE MentalEngine)
add_test(NAME MentalEngineEditorApplicationTests COMMAND MentalEngineEditorApplicationTests)

add_executable(MentalEngineRenderSceneDataTests tests/renderSceneDataTests.cpp)
target_include_directories(MentalEngineRenderSceneDataTests PRIVATE include)
target_link_libraries(MentalEngineRenderSceneDataTests PRIVATE glm)
add_test(NAME MentalEngineRenderSceneDataTests COMMAND MentalEngineRenderSceneDataTests)

add_executable(MentalEngineRenderHostAdapterTests tests/renderHostAdapterTests.cpp)
target_link_libraries(MentalEngineRenderHostAdapterTests PRIVATE MentalEngine)
target_compile_definitions(MentalEngineRenderHostAdapterTests PRIVATE MENTAL_WITH_VULKAN)
add_test(NAME MentalEngineRenderHostAdapterTests COMMAND MentalEngineRenderHostAdapterTests)

if(WIN32)
add_custom_command(
  TARGET MentalEngineSceneTests
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MENTAL_SLANG_DLL}" "$<TARGET_FILE_DIR:MentalEngineSceneTests>/slang.dll"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MENTAL_SLANG_RT_DLL}" "$<TARGET_FILE_DIR:MentalEngineSceneTests>/slang-rt.dll"
  VERBATIM
)
add_custom_command(
  TARGET MentalEngineEditorApplicationTests
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MENTAL_SLANG_DLL}" "$<TARGET_FILE_DIR:MentalEngineEditorApplicationTests>/slang.dll"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MENTAL_SLANG_RT_DLL}" "$<TARGET_FILE_DIR:MentalEngineEditorApplicationTests>/slang-rt.dll"
  VERBATIM
)
add_custom_command(
  TARGET MentalEngineRenderSceneDataTests
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MENTAL_SLANG_DLL}" "$<TARGET_FILE_DIR:MentalEngineRenderSceneDataTests>/slang.dll"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MENTAL_SLANG_RT_DLL}" "$<TARGET_FILE_DIR:MentalEngineRenderSceneDataTests>/slang-rt.dll"
  VERBATIM
)
add_custom_command(
  TARGET MentalEngineRenderHostAdapterTests
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MENTAL_SLANG_DLL}" "$<TARGET_FILE_DIR:MentalEngineRenderHostAdapterTests>/slang.dll"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MENTAL_SLANG_RT_DLL}" "$<TARGET_FILE_DIR:MentalEngineRenderHostAdapterTests>/slang-rt.dll"
  VERBATIM
)
endif()

set(MENTAL_SLANG_DLL "${MENTAL_SLANG_DLL}" CACHE INTERNAL "Path to the Slang runtime DLL")
set(MENTAL_SLANG_RT_DLL "${MENTAL_SLANG_RT_DLL}" CACHE INTERNAL "Path to the Slang runtime support DLL")

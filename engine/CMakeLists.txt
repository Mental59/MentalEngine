cmake_minimum_required(VERSION 3.30)

project(MentalEngine LANGUAGES CXX VERSION 0.0.1)

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS include/*.hpp)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})

add_library(MentalEngine STATIC ${SOURCES} ${HEADERS})

find_package(Vulkan REQUIRED)

set(MENTAL_SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(MENTAL_SHADER_SOURCE_DIR "${MENTAL_SHADER_SOURCE_DIR}" CACHE INTERNAL "Path to MentalEngine runtime shader assets")

if(WIN32)
target_compile_definitions(MentalEngine PRIVATE MENTAL_WIN32 VK_USE_PLATFORM_WIN32_KHR)
elseif(LINUX)
target_compile_definitions(MentalEngine PRIVATE MENTAL_LINUX VK_USE_PLATFORM_XCB_KHR)
else()
message(FATAL_ERROR "Unsupported platform")
endif()

target_compile_definitions(MentalEngine PRIVATE MENTAL_RUNTIME_SHADER_ROOT="./shaders")

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
set(GLFW_INSTALL OFF CACHE BOOL "")

set(GLM_BUILD_TESTS OFF CACHE BOOL "")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "")

add_subdirectory(deps/glfw)
add_subdirectory(deps/entt)
add_subdirectory(deps/glm)
add_subdirectory(deps/volk)

add_library(slang STATIC IMPORTED)
if(WIN32)
set_target_properties(slang PROPERTIES
  IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/libs/slang/windows/x86_64/slang.lib
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/deps/slang
)
elseif(LINUX)
set_target_properties(slang PROPERTIES
  IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/libs/slang/linux/x86_64/libslang.so
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/deps/slang
)
endif()

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

target_link_libraries(MentalEngine
  PUBLIC glm EnTT::EnTT volk_headers
  PRIVATE Vulkan::Vulkan glfw slang imgui
)
target_compile_definitions(MentalEngine PRIVATE MENTAL_WITH_VULKAN VK_NO_PROTOTYPES VMA_STATIC_VULKAN_FUNCTIONS=0 VMA_DYNAMIC_VULKAN_FUNCTIONS=0 NOMINMAX)
target_include_directories(MentalEngine PUBLIC include)

add_executable(MentalEngineSceneTests tests/editorSceneTests.cpp)
target_link_libraries(MentalEngineSceneTests PRIVATE MentalEngine)
add_test(NAME MentalEngineSceneTests COMMAND MentalEngineSceneTests WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_executable(MentalEngineCameraTests tests/cameraTests.cpp src/camera/camera.cpp)
target_include_directories(MentalEngineCameraTests PRIVATE include)
target_link_libraries(MentalEngineCameraTests PRIVATE glm)
add_test(NAME MentalEngineCameraTests COMMAND MentalEngineCameraTests WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_executable(MentalEngineSceneCameraTests tests/sceneCameraTests.cpp)
target_link_libraries(MentalEngineSceneCameraTests PRIVATE MentalEngine)
add_test(NAME MentalEngineSceneCameraTests COMMAND MentalEngineSceneCameraTests WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_executable(MentalEngineEditorApplicationTests tests/editorApplicationTests.cpp)
target_link_libraries(MentalEngineEditorApplicationTests PRIVATE MentalEngine)
add_test(NAME MentalEngineEditorApplicationTests COMMAND MentalEngineEditorApplicationTests WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_executable(MentalEngineRenderSceneDataTests tests/renderSceneDataTests.cpp)
target_include_directories(MentalEngineRenderSceneDataTests PRIVATE include)
target_link_libraries(MentalEngineRenderSceneDataTests PRIVATE glm)
add_test(NAME MentalEngineRenderSceneDataTests COMMAND MentalEngineRenderSceneDataTests WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_executable(MentalEnginePrimitiveMeshDataTests
  tests/primitiveMeshDataTests.cpp
  src/render/primitiveMeshData.cpp
)
target_include_directories(MentalEnginePrimitiveMeshDataTests PRIVATE include)
target_link_libraries(MentalEnginePrimitiveMeshDataTests PRIVATE glm)
add_test(NAME MentalEnginePrimitiveMeshDataTests COMMAND MentalEnginePrimitiveMeshDataTests WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_executable(MentalEngineRenderHostAdapterTests tests/renderHostAdapterTests.cpp)
target_link_libraries(MentalEngineRenderHostAdapterTests PRIVATE MentalEngine)
target_compile_definitions(MentalEngineRenderHostAdapterTests PRIVATE MENTAL_WITH_VULKAN)
add_test(NAME MentalEngineRenderHostAdapterTests COMMAND MentalEngineRenderHostAdapterTests WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_executable(MentalEngineShaderCompilerTests tests/shaderCompilerTests.cpp)
target_link_libraries(MentalEngineShaderCompilerTests PRIVATE MentalEngine)
add_test(NAME MentalEngineShaderCompilerTests COMMAND MentalEngineShaderCompilerTests WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_custom_command(
  TARGET MentalEngineShaderCompilerTests
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${MENTAL_SHADER_SOURCE_DIR}" "$<TARGET_FILE_DIR:MentalEngineShaderCompilerTests>/shaders"
  VERBATIM
)

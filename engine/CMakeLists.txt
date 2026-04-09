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
target_compile_definitions(MentalEngine PRIVATE MENTAL_LINUX)
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

add_library(slang::compiler SHARED IMPORTED)
if(WIN32)
set(MENTAL_SLANG_SHARED_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libs/slang/windows/x86_64/slang-compiler.dll" CACHE INTERNAL "")
set_target_properties(slang::compiler PROPERTIES
  IMPORTED_LOCATION ${MENTAL_SLANG_SHARED_LIB_PATH}
  IMPORTED_IMPLIB ${CMAKE_CURRENT_SOURCE_DIR}/libs/slang/windows/x86_64/slang-compiler.lib
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/deps/slang
)
elseif(LINUX)
set(MENTAL_SLANG_SHARED_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libs/slang/linux/x86_64/libslang-compiler.so.0.2026.5.2" CACHE INTERNAL "")
set_target_properties(slang::compiler PROPERTIES
  IMPORTED_LOCATION ${MENTAL_SLANG_SHARED_LIB_PATH}
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
  PRIVATE Vulkan::Vulkan glfw slang::compiler imgui
)
target_compile_definitions(MentalEngine
  PUBLIC MENTAL_WITH_VULKAN VK_NO_PROTOTYPES GLM_FORCE_DEPTH_ZERO_TO_ONE
  PRIVATE VMA_STATIC_VULKAN_FUNCTIONS=0 VMA_DYNAMIC_VULKAN_FUNCTIONS=0 NOMINMAX
)
target_include_directories(MentalEngine PUBLIC include)

add_executable(MentalEngineSceneTests tests/editorSceneTests.cpp)
target_link_libraries(MentalEngineSceneTests PRIVATE MentalEngine)
add_test(NAME MentalEngineSceneTests COMMAND MentalEngineSceneTests WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
mental_copy_shared_libs(MentalEngineSceneTests ${MENTAL_SLANG_SHARED_LIB_PATH})

add_executable(MentalEngineCameraTests tests/cameraTests.cpp src/camera/camera.cpp)
target_include_directories(MentalEngineCameraTests PRIVATE include)
target_link_libraries(MentalEngineCameraTests PRIVATE glm)
add_test(NAME MentalEngineCameraTests COMMAND MentalEngineCameraTests WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
mental_copy_shared_libs(MentalEngineCameraTests ${MENTAL_SLANG_SHARED_LIB_PATH})

add_executable(MentalEngineSceneCameraTests tests/sceneCameraTests.cpp)
target_link_libraries(MentalEngineSceneCameraTests PRIVATE MentalEngine)
add_test(NAME MentalEngineSceneCameraTests COMMAND MentalEngineSceneCameraTests WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
mental_copy_shared_libs(MentalEngineSceneTests ${MENTAL_SLANG_SHARED_LIB_PATH})

add_executable(MentalEngineEditorApplicationTests tests/editorApplicationTests.cpp)
target_link_libraries(MentalEngineEditorApplicationTests PRIVATE MentalEngine)
add_test(NAME MentalEngineEditorApplicationTests COMMAND MentalEngineEditorApplicationTests WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
mental_copy_shared_libs(MentalEngineEditorApplicationTests ${MENTAL_SLANG_SHARED_LIB_PATH})

add_executable(MentalEngineRenderSceneDataTests tests/renderSceneDataTests.cpp)
target_include_directories(MentalEngineRenderSceneDataTests PRIVATE include)
target_link_libraries(MentalEngineRenderSceneDataTests PRIVATE glm)
add_test(NAME MentalEngineRenderSceneDataTests COMMAND MentalEngineRenderSceneDataTests WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
mental_copy_shared_libs(MentalEngineRenderSceneDataTests ${MENTAL_SLANG_SHARED_LIB_PATH})

add_executable(MentalEnginePrimitiveMeshDataTests
  tests/primitiveMeshDataTests.cpp
  src/render/primitiveMeshData.cpp
)
target_include_directories(MentalEnginePrimitiveMeshDataTests PRIVATE include)
target_link_libraries(MentalEnginePrimitiveMeshDataTests PRIVATE glm)
add_test(NAME MentalEnginePrimitiveMeshDataTests COMMAND MentalEnginePrimitiveMeshDataTests WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
mental_copy_shared_libs(MentalEnginePrimitiveMeshDataTests ${MENTAL_SLANG_SHARED_LIB_PATH})

add_executable(MentalEngineRenderHostAdapterTests tests/renderHostAdapterTests.cpp)
target_link_libraries(MentalEngineRenderHostAdapterTests PRIVATE MentalEngine)
target_compile_definitions(MentalEngineRenderHostAdapterTests PRIVATE MENTAL_WITH_VULKAN)
add_test(NAME MentalEngineRenderHostAdapterTests COMMAND MentalEngineRenderHostAdapterTests WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
mental_copy_shared_libs(MentalEngineRenderHostAdapterTests ${MENTAL_SLANG_SHARED_LIB_PATH})

add_executable(MentalEngineShaderCompilerTests tests/shaderCompilerTests.cpp)
target_link_libraries(MentalEngineShaderCompilerTests PRIVATE MentalEngine)
add_test(NAME MentalEngineShaderCompilerTests COMMAND MentalEngineShaderCompilerTests WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_custom_command(
  TARGET MentalEngineShaderCompilerTests
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${MENTAL_SHADER_SOURCE_DIR}" "$<TARGET_FILE_DIR:MentalEngineShaderCompilerTests>/shaders"
  VERBATIM
)
mental_copy_shared_libs(MentalEngineShaderCompilerTests ${MENTAL_SLANG_SHARED_LIB_PATH})

add_executable(MentalEngineCoreTests tests/coreTests.cpp src/core/log.cpp src/core/types.cpp)
target_include_directories(MentalEngineCoreTests PRIVATE include)
target_compile_definitions(MentalEngineCoreTests PRIVATE MENTAL_LOG_INFO MENTAL_LOG_WARNINGS MENTAL_LOG_TRACES)
add_test(NAME MentalEngineCoreTests COMMAND MentalEngineCoreTests WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
mental_copy_shared_libs(MentalEngineCoreTests ${MENTAL_SLANG_SHARED_LIB_PATH})

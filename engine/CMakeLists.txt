cmake_minimum_required(VERSION 3.30)

project(MentalEngine LANGUAGES CXX VERSION 0.0.1)

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS include/*.hpp)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})

add_library(MentalEngine STATIC ${SOURCES} ${HEADERS})

find_package(Vulkan REQUIRED COMPONENTS volk)

set(MENTAL_SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(MENTAL_SHADER_SOURCE_DIR "${MENTAL_SHADER_SOURCE_DIR}" CACHE INTERNAL "Path to MentalEngine runtime shader assets")

set(MENTAL_VULKAN_SDK "$ENV{VULKAN_SDK}")
if(NOT MENTAL_VULKAN_SDK)
  message(FATAL_ERROR "VULKAN_SDK environment variable is not set")
endif()

if(WIN32)
target_compile_definitions(MentalEngine PRIVATE MENTAL_WIN32 VK_USE_PLATFORM_WIN32_KHR)
set(MENTAL_SLANG_LINK_LIB "${MENTAL_VULKAN_SDK}/Lib/slang.lib")
elseif(LINUX)
target_compile_definitions(MentalEngine PRIVATE MENTAL_LINUX VK_USE_PLATFORM_XCB_KHR)
set(MENTAL_SLANG_LINK_LIB "${MENTAL_VULKAN_SDK}/lib/libslang.so")
set(MENTAL_LINUX_RUNTIME_LIBS "${MENTAL_SLANG_LINK_LIB}" CACHE INTERNAL "Linux runtime libraries")
else()
message(FATAL_ERROR "Unsupported platform")
endif()

set(MENTAL_GLM_INCLUDE_DIR "${Vulkan_INCLUDE_DIR}")
set(MENTAL_SLANG_INCLUDE_DIR "${Vulkan_INCLUDE_DIR}")

foreach(MENTAL_REQUIRED_PATH
"${MENTAL_GLM_INCLUDE_DIR}/glm"
"${MENTAL_SLANG_INCLUDE_DIR}/slang"
"${MENTAL_SLANG_LINK_LIB}")
if(NOT EXISTS "${MENTAL_REQUIRED_PATH}")
message(FATAL_ERROR "Required Vulkan SDK dependency path is missing: ${MENTAL_REQUIRED_PATH}")
endif()
endforeach()

foreach(MENTAL_LINUX_RUNTIME_LIB ${MENTAL_LINUX_RUNTIME_LIBS})
if(NOT EXISTS "${MENTAL_LINUX_RUNTIME_LIB}")
message(FATAL_ERROR "Required Vulkan SDK runtime path is missing: ${MENTAL_LINUX_RUNTIME_LIB}")
endif()
endforeach()

target_compile_definitions(MentalEngine PRIVATE MENTAL_RUNTIME_SHADER_ROOT="./shaders")

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
set(GLFW_INSTALL OFF CACHE BOOL "")

add_subdirectory(deps/glfw)
add_subdirectory(deps/entt)

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE "${MENTAL_GLM_INCLUDE_DIR}")

if(WIN32)
add_library(slang STATIC IMPORTED)
else()
add_library(slang SHARED IMPORTED)
endif()
set_target_properties(slang PROPERTIES
  IMPORTED_LOCATION "${MENTAL_SLANG_LINK_LIB}"
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
target_link_libraries(imgui PRIVATE glfw Vulkan::Vulkan Vulkan::volk)
target_compile_definitions(imgui PRIVATE IMGUI_IMPL_VULKAN_NO_PROTOTYPES NOMINMAX)

target_include_directories(MentalEngine PUBLIC include ${Vulkan_INCLUDE_DIRS})
target_link_libraries(MentalEngine
  PUBLIC glm EnTT::EnTT
  PRIVATE Vulkan::Vulkan Vulkan::volk glfw slang imgui
)
target_compile_definitions(MentalEngine PRIVATE MENTAL_WITH_VULKAN VK_NO_PROTOTYPES VMA_STATIC_VULKAN_FUNCTIONS=0 VMA_DYNAMIC_VULKAN_FUNCTIONS=0 NOMINMAX)

add_executable(MentalEngineSceneTests tests/editorSceneTests.cpp)
target_link_libraries(MentalEngineSceneTests PRIVATE MentalEngine)
add_test(NAME MentalEngineSceneTests COMMAND MentalEngineSceneTests WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
mental_copy_linux_runtime_libraries(MentalEngineSceneTests "${MENTAL_LINUX_RUNTIME_LIBS}")

add_executable(MentalEngineCameraTests tests/cameraTests.cpp src/camera/camera.cpp)
target_include_directories(MentalEngineCameraTests PRIVATE include)
target_link_libraries(MentalEngineCameraTests PRIVATE glm)
add_test(NAME MentalEngineCameraTests COMMAND MentalEngineCameraTests WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
mental_copy_linux_runtime_libraries(MentalEngineCameraTests "${MENTAL_LINUX_RUNTIME_LIBS}")

add_executable(MentalEngineSceneCameraTests tests/sceneCameraTests.cpp)
target_link_libraries(MentalEngineSceneCameraTests PRIVATE MentalEngine)
add_test(NAME MentalEngineSceneCameraTests COMMAND MentalEngineSceneCameraTests WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
mental_copy_linux_runtime_libraries(MentalEngineSceneCameraTests "${MENTAL_LINUX_RUNTIME_LIBS}")

add_executable(MentalEngineEditorApplicationTests tests/editorApplicationTests.cpp)
target_link_libraries(MentalEngineEditorApplicationTests PRIVATE MentalEngine)
add_test(NAME MentalEngineEditorApplicationTests COMMAND MentalEngineEditorApplicationTests WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
mental_copy_linux_runtime_libraries(MentalEngineEditorApplicationTests "${MENTAL_LINUX_RUNTIME_LIBS}")

add_executable(MentalEngineRenderSceneDataTests tests/renderSceneDataTests.cpp)
target_include_directories(MentalEngineRenderSceneDataTests PRIVATE include)
target_link_libraries(MentalEngineRenderSceneDataTests PRIVATE glm)
add_test(NAME MentalEngineRenderSceneDataTests COMMAND MentalEngineRenderSceneDataTests WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
mental_copy_linux_runtime_libraries(MentalEngineRenderSceneDataTests "${MENTAL_LINUX_RUNTIME_LIBS}")

add_executable(MentalEnginePrimitiveMeshDataTests
  tests/primitiveMeshDataTests.cpp
  src/render/primitiveMeshData.cpp
)
target_include_directories(MentalEnginePrimitiveMeshDataTests PRIVATE include)
target_link_libraries(MentalEnginePrimitiveMeshDataTests PRIVATE glm)
add_test(NAME MentalEnginePrimitiveMeshDataTests COMMAND MentalEnginePrimitiveMeshDataTests WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
mental_copy_linux_runtime_libraries(MentalEnginePrimitiveMeshDataTests "${MENTAL_LINUX_RUNTIME_LIBS}")

add_executable(MentalEngineRenderHostAdapterTests tests/renderHostAdapterTests.cpp)
target_link_libraries(MentalEngineRenderHostAdapterTests PRIVATE MentalEngine)
target_compile_definitions(MentalEngineRenderHostAdapterTests PRIVATE MENTAL_WITH_VULKAN)
add_test(NAME MentalEngineRenderHostAdapterTests COMMAND MentalEngineRenderHostAdapterTests WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
mental_copy_linux_runtime_libraries(MentalEngineRenderHostAdapterTests "${MENTAL_LINUX_RUNTIME_LIBS}")

add_executable(MentalEngineShaderCompilerTests tests/shaderCompilerTests.cpp)
target_link_libraries(MentalEngineShaderCompilerTests PRIVATE MentalEngine)
add_test(NAME MentalEngineShaderCompilerTests COMMAND MentalEngineShaderCompilerTests WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
mental_copy_linux_runtime_libraries(MentalEngineShaderCompilerTests "${MENTAL_LINUX_RUNTIME_LIBS}")
add_custom_command(
  TARGET MentalEngineShaderCompilerTests
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${MENTAL_SHADER_SOURCE_DIR}" "$<TARGET_FILE_DIR:MentalEngineShaderCompilerTests>/shaders"
  VERBATIM
)

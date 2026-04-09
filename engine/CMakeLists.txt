cmake_minimum_required(VERSION 3.30)

project(MentalEngine LANGUAGES CXX VERSION 0.0.1)

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS include/*.hpp)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})

add_library(MentalEngine STATIC ${SOURCES} ${HEADERS})

find_package(Vulkan REQUIRED)

set(MENTAL_GENERATED_SHADER_DIR "${CMAKE_BINARY_DIR}/generated/shaders")
set(MENTAL_RUNTIME_SHADER_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders")

if(WIN32)
set(MENTAL_SLANGC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/slangCompiler/windows/slangc.exe")
set(MENTAL_SLANGC_RUNTIME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/slangCompiler/windows")
elseif(LINUX)
set(MENTAL_SLANGC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/slangCompiler/linux/slangc")
set(MENTAL_SLANGC_RUNTIME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/slangCompiler/linux")
else()
message(FATAL_ERROR "Unsupported platform")
endif()

if(NOT EXISTS "${MENTAL_SLANGC_PATH}")
message(FATAL_ERROR "Missing slangc executable at ${MENTAL_SLANGC_PATH}")
endif()

function(mental_compile_slang_shader outVar shaderFile entryPoint stage outputFile)
set(outputPath "${MENTAL_GENERATED_SHADER_DIR}/${outputFile}")
get_filename_component(outputDir "${outputPath}" DIRECTORY)

if(WIN32)
  add_custom_command(
    OUTPUT "${outputPath}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${outputDir}"
    COMMAND ${CMAKE_COMMAND} -E env "PATH=${MENTAL_SLANGC_RUNTIME_DIR};$ENV{PATH}"
      "${MENTAL_SLANGC_PATH}"
      "${CMAKE_CURRENT_SOURCE_DIR}/shaders/${shaderFile}"
      -target spirv
      -profile sm_6_5
      -entry "${entryPoint}"
      -stage "${stage}"
      -matrix-layout-column-major
      -force-glsl-scalar-layout
      -fvk-use-entrypoint-name
      -o "${outputPath}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/${shaderFile}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
    VERBATIM
  )
elseif(LINUX)
  add_custom_command(
    OUTPUT "${outputPath}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${outputDir}"
    COMMAND ${CMAKE_COMMAND} -E env "LD_LIBRARY_PATH=${MENTAL_SLANGC_RUNTIME_DIR}:$ENV{LD_LIBRARY_PATH}"
      "${MENTAL_SLANGC_PATH}"
      "${CMAKE_CURRENT_SOURCE_DIR}/shaders/${shaderFile}"
      -target spirv
      -profile sm_6_5
      -entry "${entryPoint}"
      -stage "${stage}"
      -matrix-layout-column-major
      -force-glsl-scalar-layout
      -fvk-use-entrypoint-name
      -o "${outputPath}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/${shaderFile}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
    VERBATIM
  )
endif()

set(${outVar} "${outputPath}" PARENT_SCOPE)
endfunction()

mental_compile_slang_shader(PRIMITIVE_VERTEX_SPV "primitiveScene.slang" "vertexMain" "vertex" "primitiveScene.vertex.spv")
mental_compile_slang_shader(PRIMITIVE_FRAGMENT_SPV "primitiveScene.slang" "fragmentMain" "fragment" "primitiveScene.fragment.spv")
mental_compile_slang_shader(GRID_VERTEX_SPV "editorGrid.slang" "vertexMain" "vertex" "editorGrid.vertex.spv")
mental_compile_slang_shader(GRID_FRAGMENT_SPV "editorGrid.slang" "fragmentMain" "fragment" "editorGrid.fragment.spv")

set(MENTAL_GENERATED_SHADERS
  "${PRIMITIVE_VERTEX_SPV}"
  "${PRIMITIVE_FRAGMENT_SPV}"
  "${GRID_VERTEX_SPV}"
  "${GRID_FRAGMENT_SPV}"
)

add_custom_target(MentalEngineShaders ALL DEPENDS ${MENTAL_GENERATED_SHADERS})

add_custom_command(
  OUTPUT "${MENTAL_RUNTIME_SHADER_DIR}/.stamp"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${MENTAL_RUNTIME_SHADER_DIR}"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${MENTAL_GENERATED_SHADER_DIR}" "${MENTAL_RUNTIME_SHADER_DIR}"
  COMMAND ${CMAKE_COMMAND} -E touch "${MENTAL_RUNTIME_SHADER_DIR}/.stamp"
  DEPENDS ${MENTAL_GENERATED_SHADERS}
  VERBATIM
)
add_custom_target(MentalRuntimeShaders ALL DEPENDS "${MENTAL_RUNTIME_SHADER_DIR}/.stamp")

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
  PRIVATE Vulkan::Vulkan glfw imgui
)
target_compile_definitions(MentalEngine
  PUBLIC MENTAL_WITH_VULKAN VK_NO_PROTOTYPES GLM_FORCE_DEPTH_ZERO_TO_ONE
  PRIVATE VMA_STATIC_VULKAN_FUNCTIONS=0 VMA_DYNAMIC_VULKAN_FUNCTIONS=0 NOMINMAX
)
target_include_directories(MentalEngine PUBLIC include)
add_dependencies(MentalEngine MentalEngineShaders)
add_dependencies(MentalEngine MentalRuntimeShaders)

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
add_dependencies(MentalEngineShaderCompilerTests MentalRuntimeShaders)
add_test(NAME MentalEngineShaderCompilerTests COMMAND MentalEngineShaderCompilerTests WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_executable(MentalEngineCoreTests tests/coreTests.cpp src/core/log.cpp src/core/types.cpp)
target_include_directories(MentalEngineCoreTests PRIVATE include)
target_compile_definitions(MentalEngineCoreTests PRIVATE MENTAL_LOG_INFO MENTAL_LOG_WARNINGS MENTAL_LOG_TRACES)
add_test(NAME MentalEngineCoreTests COMMAND MentalEngineCoreTests WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

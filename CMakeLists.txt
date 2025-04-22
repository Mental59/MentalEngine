cmake_minimum_required(VERSION 3.30)

include(cmake/CMakeHelpers.cmake)

project(MentalEngine)

setup_app(MentalEngine 0.0.1)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
set(GLFW_INSTALL OFF CACHE BOOL "")

set(EASY_PROFILER_NO_GUI ON CACHE BOOL "")
set(EASY_PROFILER_NO_SAMPLES ON CACHE BOOL "")

set(ASSIMP_NO_EXPORT ON CACHE BOOL "")
set(ASSIMP_BUILD_DRACO OFF CACHE BOOL "")
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "")
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "")
set(ASSIMP_INSTALL_PDB OFF CACHE BOOL "")
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "")
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "")
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "")

set(MESHOPT_BUILD_DEMO OFF CACHE BOOL "")
set(MESHOPT_BUILD_TOOLS OFF CACHE BOOL "")
set(MESHOPT_BUILD_SHARED_LIBS OFF CACHE BOOL "")

set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "")
set(ENABLE_HLSL OFF CACHE BOOL "")
set(ENABLE_CTEST OFF CACHE BOOL "")
set(ENABLE_OPT OFF CACHE BOOL "")
set(SKIP_GLSLANG_INSTALL ON CACHE BOOL "")

if(WIN32)
target_compile_definitions(MentalEngine PRIVATE VK_USE_PLATFORM_WIN32_KHR)
elseif(LINUX)
target_compile_definitions(MentalEngine PRIVATE VK_USE_PLATFORM_XCB_KHR)
elseif(APPLE)
target_compile_definitions(MentalEngine PRIVATE VK_USE_PLATFORM_METAL_EXT)
else()
message(FATAL_ERROR "Unsupported platform")
endif()

find_package(Vulkan REQUIRED)

add_subdirectory(deps/libs/glfw)
add_subdirectory(deps/libs/glm)
add_subdirectory(deps/libs/volk)
add_subdirectory(deps/libs/assimp)
add_subdirectory(deps/libs/optick)
add_subdirectory(deps/libs/glslang)

set_property(TARGET assimp PROPERTY FOLDER "Assimp")
set_property(TARGET uninstall PROPERTY FOLDER "Assimp")
set_property(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTY FOLDER "Assimp")
set_property(TARGET zlibstatic PROPERTY FOLDER "Assimp")
set_property(TARGET volk PROPERTY FOLDER "Volk")
set_property(TARGET optick PROPERTY FOLDER "Optick")
set_property(TARGET glm PROPERTY FOLDER "GLM")

target_link_libraries(MentalEngine PRIVATE glm glfw assimp volk glslang SPIRV)
target_compile_definitions(MentalEngine PRIVATE VK_NO_PROTOTYPES GLFW_INCLUDE_VULKAN)
target_include_directories(${PROJECT_NAME} PRIVATE deps/libs/stb)

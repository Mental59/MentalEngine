cmake_minimum_required(VERSION 3.30)

include(cmake/CMakeHelpers.cmake)
setup_conan()

project(MentalEngine)

setup_app(MentalEngine 0.0.1)

find_package(glm REQUIRED)
find_package(glfw3 3.4 REQUIRED)
find_package(stb REQUIRED)
find_package(assimp REQUIRED)
find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)

if(WIN32)
target_compile_definitions(MentalEngine PRIVATE VK_USE_PLATFORM_WIN32_KHR)
elseif(LINUX)
target_compile_definitions(MentalEngine PRIVATE VK_USE_PLATFORM_XCB_KHR)
elseif(APPLE)
target_compile_definitions(MentalEngine PRIVATE VK_USE_PLATFORM_METAL_EXT)
else()
message(FATAL_ERROR "Unsupported platform")
endif()

add_subdirectory(deps/libs/volk)

target_link_libraries(
  MentalEngine PRIVATE
  glm::glm
  glfw
  stb::stb
  assimp::assimp
  volk_headers
  Vulkan::shaderc_combined
)

target_compile_definitions(MentalEngine PRIVATE VK_NO_PROTOTYPES GLFW_INCLUDE_VULKAN)

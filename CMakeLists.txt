cmake_minimum_required(VERSION 3.30)

project(MentalEngine)

include(cmake/CMakeHelpers.cmake)

SETUP_APP(MentalEngine 0.0.1)

find_package(glm REQUIRED)
find_package(glfw3 3.4 REQUIRED)
find_package(stb REQUIRED)
# find_package(imgui REQUIRED)
# find_package(easy_profiler REQUIRED)
find_package(assimp REQUIRED)
# find_package(Taskflow REQUIRED)
# find_package(meshoptimizer REQUIRED)

# add_subdirectory(deps/libs/optick)
# add_subdirectory(deps/libs/etc2comp)

target_link_libraries(
  MentalEngine PRIVATE
  glm::glm
  glfw
  stb::stb
  assimp::assimp
  # imgui::imgui
  # easy_profiler
  # optick
  # EtcTool
  # Taskflow::Taskflow
  # meshoptimizer::meshoptimizer
)

# target_compile_definitions(MentalEngine PRIVATE BUILD_WITH_EASY_PROFILER)
# target_include_directories(
  # ${PROJECT_NAME} PRIVATE
  # deps/libs
  # deps/libs/optick
  # deps/libs/etc2comp/EtcLib/Etc
  # deps/libs/etc2comp/EtcLib/EtcCodec
# )

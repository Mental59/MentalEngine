cmake_minimum_required(VERSION 3.30)

macro(setup_app projname version)
  set(PROJECT_NAME ${projname})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

  project(${PROJECT_NAME} VERSION ${version})

  file(GLOB_RECURSE SOURCES src/*.cpp)
  file(GLOB_RECURSE HEADERS src/*.hpp src/*.h)
  file(GLOB_RECURSE SHADERS src/*.frag src/*.vert)

  source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS} ${SHADERS})

  add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${SHADERS})

  set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME_DEBUG ${PROJECT_NAME}${version}_Debug)
	set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME_RELEASE ${PROJECT_NAME}${version}_Release)
	set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME_RELWITHDEBINFO ${PROJECT_NAME}${version}_ReleaseDebInfo)

  target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/deps/includes)

  target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

  file(GLOB_RECURSE DEPS_SOURCES deps/sources/*.cpp deps/sources/*.c)
  target_sources(MentalEngine PRIVATE ${DEPS_SOURCES})

  if(MSVC)
    unset(CMAKE_INCLUDE_SYSTEM_FLAG_CXX)
    unset(_CMAKE_INCLUDE_SYSTEM_FLAG_CXX_WARNING)
		set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
	endif()
endmacro()

macro(setup_imgui)
  set(IMGUI_SOURCES
    deps/libs/imgui/imgui.cpp
    deps/libs/imgui/imgui_demo.cpp
    deps/libs/imgui/imgui_draw.cpp
    deps/libs/imgui/imgui_tables.cpp
    deps/libs/imgui/imgui_widgets.cpp
    deps/libs/imgui/backends/imgui_impl_glfw.cpp
    deps/libs/imgui/backends/imgui_impl_opengl3.cpp
    deps/libs/imgui/backends/imgui_impl_vulkan.cpp
  )

  add_library(imgui STATIC ${IMGUI_SOURCES})

  target_include_directories(imgui PUBLIC deps/libs/imgui deps/libs/imgui/backends)
  target_compile_definitions(imgui PRIVATE IMGUI_IMPL_VULKAN_USE_VOLK)
  target_link_libraries(imgui PRIVATE volk glfw)
endmacro()

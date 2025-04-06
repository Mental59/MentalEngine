cmake_minimum_required(VERSION 3.30)

macro(setup_conan)
  # setup conan profiles
  set(PROFILE_PATH_Windows_Release "${CMAKE_CURRENT_SOURCE_DIR}/conan/profiles/windows/release.txt")
  set(PROFILE_PATH_Windows_Debug "${CMAKE_CURRENT_SOURCE_DIR}/conan/profiles/windows/debug.txt")
  set(PROFILE_PATH_Linux_Release "${CMAKE_CURRENT_SOURCE_DIR}/conan/profiles/linux/release.txt")
  set(PROFILE_PATH_Linux_Debug "${CMAKE_CURRENT_SOURCE_DIR}/conan/profiles/linux/debug.txt")

  set(CURRENT_PROFILE "PROFILE_PATH_${CMAKE_HOST_SYSTEM_NAME}_${CMAKE_BUILD_TYPE}")
  message(STATUS "Set conan profile: ${CURRENT_PROFILE}")

  if(DEFINED ${CURRENT_PROFILE})
    set(CONAN_PROFILE "${${CURRENT_PROFILE}}")
    message(STATUS "Using conan profile: ${CONAN_PROFILE}")
  else()
      message(FATAL_ERROR "Unsupported configuration for: ${CURRENT_PROFILE}")
  endif()

  set(CMAKE_CONAN_PATH "${CMAKE_BINARY_DIR}/conan_provider.cmake")
  if(NOT EXISTS ${CMAKE_CONAN_PATH})
      message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
      file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/refs/heads/develop2/conan_provider.cmake" "${CMAKE_CONAN_PATH}")
  endif()

  set(CMAKE_PROJECT_TOP_LEVEL_INCLUDES ${CMAKE_CONAN_PATH})
  set(CONAN_HOST_PROFILE "${CONAN_PROFILE}")
  set(CONAN_BUILD_PROFILE "${CONAN_PROFILE}")
endmacro()

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

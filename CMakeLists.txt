cmake_minimum_required(VERSION 3.30)

project(MentalRoot LANGUAGES CXX VERSION 0.0.1)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

function(mental_copy_shared_libs targetName)
  foreach(sharedLib IN LISTS ARGN)
    add_custom_command(
      TARGET ${targetName}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${sharedLib}" "$<TARGET_FILE_DIR:${targetName}>"
      VERBATIM
    )
  endforeach()

  if(LINUX)
    set_property(TARGET ${targetName} APPEND PROPERTY BUILD_RPATH "\$ORIGIN")
  endif()
endfunction()

enable_testing()

if(MSVC)
  unset(CMAKE_INCLUDE_SYSTEM_FLAG_CXX)
  unset(_CMAKE_INCLUDE_SYSTEM_FLAG_CXX_WARNING)
endif()

add_subdirectory(engine)
add_subdirectory(editor)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MentalEditor)

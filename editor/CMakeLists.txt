cmake_minimum_required(VERSION 3.30)
project(MentalEditor LANGUAGES CXX VERSION 0.0.1)

option(LOG_WARNINGS "Enable warning logs" ON)
option(LOG_INFO "Enable info logs" ON)
option(LOG_TRACES "Enable trace logs" ON)
option(ENABLE_ASSERTS "Enable asserts" ON)

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS include/*.hpp)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})

add_executable(MentalEditor ${SOURCES} ${HEADERS})

target_link_libraries(MentalEditor PRIVATE MentalEngine)

if(LOG_WARNINGS)
target_compile_definitions(MentalEngine PRIVATE MENTAL_LOG_WARNINGS)
endif()
if(LOG_INFO)
target_compile_definitions(MentalEngine PRIVATE MENTAL_LOG_INFO)
endif()
if(LOG_TRACES)
target_compile_definitions(MentalEngine PRIVATE MENTAL_LOG_TRACES)
endif()
if(ENABLE_ASSERTS)
target_compile_definitions(MentalEngine PRIVATE MENTAL_ASSERTS)
endif()


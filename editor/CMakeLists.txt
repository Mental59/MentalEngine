cmake_minimum_required(VERSION 3.30)
project(MentalEditor LANGUAGES CXX VERSION 0.0.1)

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS include/*.hpp)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})

add_executable(MentalEditor ${SOURCES} ${HEADERS})

target_link_libraries(MentalEditor PRIVATE MentalEngine)

file(GLOB_RECURSE SOURCES *.cpp)
file(GLOB_RECURSE HEADERS *.h)

add_library(optick STATIC ${SOURCES} ${HEADERS})

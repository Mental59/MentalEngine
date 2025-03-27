set "folderPath=build\shaders"

if not exist "%folderPath%" (
    mkdir "%folderPath%"
    echo Folder created: %folderPath%
)

glslc.exe src/shaders/base.vert -o build/shaders/baseVert.spv -O
glslc.exe src/shaders/base.frag -o build/shaders/baseFrag.spv -O

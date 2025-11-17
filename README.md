# MentalEngine

A hobby game engine built with C++ and Vulkan.

## Features
- Vulkan-based rendering
- Modern C++ architecture
- Cross-platform support

## Requirements
- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home)
- C++20 compiler (Clang recommended)
- CMake 3.30+

## Cloning and Building
```bash
git clone --recursive https://github.com/Mental59/MentalEngine.git
cd MentalEngine
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

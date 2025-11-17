# MentalEngine

A hobby game engine built with C++ and Vulkan.

## Features
- Vulkan-based rendering
- Modern C++ architecture
- Cross-platform support

## Requirements
- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home)
- C++20 compiler ([Clang 21.1.5](https://github.com/llvm/llvm-project/releases/tag/llvmorg-21.1.5) recommended)
- [CMake 3.30+](https://cmake.org/download/)

## Cloning and Building
```bash
git clone --recursive https://github.com/Mental59/MentalEngine.git
cd MentalEngine
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

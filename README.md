# MentalEngine

MentalEngine is an early-stage C++20 game engine project with a desktop editor executable and a Vulkan-based rendering stack.

The repository currently builds two targets:

- `MentalEngine`: a static engine library
- `MentalEditor`: a desktop executable that creates a window and runs the render loop

## Current Scope

This project is still in foundation work rather than feature-complete engine territory. The codebase currently focuses on:

- platform window creation
- render system bootstrap
- a Vulkan-backed rendering hardware interface (RHI)
- per-frame resource management utilities
- logging, assertions, and basic engine core types

The README intentionally describes the project as it exists today, not as a full engine roadmap.

## Requirements

To configure and build the project you currently need:

- CMake `3.30+`
- a C++20-capable compiler
- Ninja
- Vulkan SDK with `VULKAN_SDK` set in the environment

The current CMake presets are configured to use Clang:

- `clang`
- `clang++`

The build also expects parts of the Vulkan SDK beyond the Vulkan loader itself:

- `glm` headers from the SDK include directory
- Slang headers and runtime files from the SDK
- `slang.lib`
- `slang.dll`
- `slang-rt.dll`

If `VULKAN_SDK` is missing, or those SDK paths are not present, configuration will fail.

## Dependencies

This repository vendors some third-party dependencies under `engine/deps`:

- GLFW
- Dear ImGui

Other dependencies are resolved from the Vulkan SDK during configuration:

- Vulkan
- GLM
- Slang

## Build

Clone the repository with submodules:

```bash
git clone --recursive https://github.com/Mental59/MentalEngine.git
cd MentalEngine
```

Configure with the provided preset:

```bash
cmake --preset debug
```

Build:

```bash
cmake --build build
```

For a release build:

```bash
cmake --preset release
cmake --build build --config Release
```

The editor executable is generated under `build/bin`.

## Run

After building, run the editor executable:

```bash
./build/bin/MentalEditor
```

On Windows, the build copies `slang.dll` and `slang-rt.dll` next to the editor executable as a post-build step.

## Architecture Overview

The project is split into a reusable engine library and a thin executable layer:

- `engine/`
  Contains the core engine code, including platform abstractions, rendering, resource management, and shared utilities.
- `editor/`
  Contains the `MentalEditor` executable entry point. Right now it mainly creates a window, initializes the render system, and drives the main loop.

Within the engine, the main areas are:

- `engine/include/core` and `engine/src/core`
  Basic shared infrastructure such as result/types, logging, and resource lifetime helpers.
- `engine/include/platform` and `engine/src/platform`
  Window abstractions and the current desktop window implementation (`PCWindow`).
- `engine/include/render` and `engine/src/render`
  The high-level render system that initializes graphics and runs frame rendering.
- `engine/include/render/rhi` and `engine/src/render/rhi`
  Rendering hardware interface abstractions for devices, swapchains, command lists, buffers, textures, and synchronization.
- `engine/include/render/rhi/vulkan` and `engine/src/render/rhi/vulkan`
  The current concrete RHI backend built on Vulkan.
- `engine/include/resource` and `engine/src/resource`
  Resource manager code for engine-owned GPU/frame resources.

In practice, the current startup path is:

1. `MentalEditor` creates a `PCWindow`
2. the render system is initialized with `GraphicsApi::Vulkan`
3. the Vulkan device/swapchain path is created through the RHI layer
4. the main loop calls `render()` until the window closes

## Project Layout

```text
MentalEngine/
|- engine/          # Static engine library
|- editor/          # Editor executable
|- scripts/         # Project helper scripts
|- CMakeLists.txt   # Root build entry
|- CMakePresets.json
```

## Notes

- The root CMake project sets `MentalEditor` as the Visual Studio startup target.
- The project is currently Vulkan-first. The RHI enum exists, but only the Vulkan backend is implemented.
- Logging and assertions are controlled through CMake options in `editor/CMakeLists.txt`.

## License

This project is licensed under the terms of the [LICENSE](LICENSE) file.

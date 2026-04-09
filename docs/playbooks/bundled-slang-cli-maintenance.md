---
source: extracted
status: draft
updated: 2026-04-09
---

# Bundled Slang CLI Maintenance

## Purpose

Keep the repo-bundled Slang CLI toolchains usable for build-time shader compilation on both Windows and Linux.

## Preconditions

- The repo vendors Slang under `engine/slangCompiler/`.
- CMake invokes the bundled `slangc` during the build to generate SPIR-V.
- You can inspect and update git-tracked file modes.

## Ordered Steps

1. Vendor complete per-platform tool folders under `engine/slangCompiler/windows` and `engine/slangCompiler/linux`, not just the top-level compiler binary.
2. On Linux, ensure `engine/slangCompiler/linux/slangc` is tracked as executable in git.
3. Keep required Slang sidecar libraries beside the compiler in the same platform folder so the runtime loader can resolve them from the configured directory.
4. On Windows, keep the required Slang DLL sidecars in `engine/slangCompiler/windows` alongside `slangc.exe` so the bundled compiler can resolve its runtime dependencies.
5. Reconfigure and rebuild the shader-generating targets so CMake exercises the bundled compiler paths.
6. If Linux CI reports missing `slang-glslang-*`, `spirv-opt`, or similar dynamic-library/downstream-compiler errors, update the vendored Linux Slang package to include the missing sidecars from the matching Slang release.

## Verification

1. Check the tracked mode for the Linux compiler:
   `git ls-files --stage engine/slangCompiler/linux/slangc`
   Expected: mode `100755`
2. Build the shader-generating targets:
   `cmake --build build --target MentalEditor MentalEngineShaderCompilerTests`
3. Run focused verification:
   `ctest --test-dir build --output-on-failure -R "MentalEngine(ShaderCompiler|EditorApplication|RenderHostAdapter)Tests"`

## Risks Or Rollback Notes

- If the Linux executable bit is missing, CI can fail immediately with `permission denied`.
- If the vendored Linux Slang package is incomplete, CI can fail with downstream compiler or missing dynamic-library errors even when the CMake graph is correct.
- If the Windows Slang bundle is incomplete, the bundled `slangc.exe` may fail to start or fail to resolve downstream runtime libraries during shader compilation.

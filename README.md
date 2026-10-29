# LongRoad Engine

A Vulkan rendering engine written in C++23, using `vulkan-hpp` RAII bindings.
Builds on Linux and Windows.

The repository is a CMake super-project split into three parts:

| Target          | Path                        | What it is                                  |
| --------------- | --------------------------- | ------------------------------------------- |
| `LongRoadEng`   | `src/engine`                | Static library — the engine itself           |
| `VulkanTutorial`| `src/games/VulkanTutorial`  | Sample executable that links the engine      |
| —               | `src/shaders`               | Shader stage (not wired up yet)              |

## Dependencies

### Vendored — nothing to install

These live in `external/` and are built from source as part of the project:

- **GLFW** — windowing and input
- **SDL3** — windowing, input, audio (3.4.18, built static)
- **GLM** — math

### Required on your machine

- **Vulkan SDK / loader + headers.** `find_package(Vulkan REQUIRED)` must
  succeed. The engine creates a **Vulkan 1.4** instance and rejects any physical
  device reporting less than **Vulkan 1.3**, so an old SDK or driver will fail at
  runtime even if it compiles.
- **A GPU + driver with `VK_KHR_swapchain`.** Every desktop Vulkan driver has it,
  but a software fallback like lavapipe still needs to advertise API 1.3+.
- **Vulkan validation layers** (`VK_LAYER_KHRONOS_validation`). Debug builds
  enable them unconditionally and **throw at startup if the layer is missing** —
  see `enableValidationLayers` in `src/engine/core/engine.h`. Release builds
  (`NDEBUG`) skip them.
- **CMake ≥ 4.3.** Set by `cmake_minimum_required` in the top-level
  `CMakeLists.txt`. This is a recent release — check `cmake --version` before
  anything else, since distro packages are often older.
- **Ninja.** Every preset uses the Ninja generator.
- **A C++23 compiler.** Clang on Linux, MSVC on Windows (see below).

## Linux

### Toolchain

The presets pin **Clang** (`CMAKE_C_COMPILER=clang`,
`CMAKE_CXX_COMPILER=clang++`). GCC will likely work too, but you would have to
configure without the preset and pass the compiler yourself.

### System packages

GLFW builds **both X11 and Wayland backends by default**, so the headers for
both are required. To skip one, pass `-DGLFW_BUILD_WAYLAND=OFF` or
`-DGLFW_BUILD_X11=OFF` at configure time.

Arch / CachyOS:

```sh
sudo pacman -S --needed \
  base-devel cmake ninja clang \
  vulkan-headers vulkan-icd-loader vulkan-validation-layers \
  libx11 libxrandr libxinerama libxcursor libxi \
  wayland wayland-protocols libxkbcommon
```

Plus the driver for your GPU: `vulkan-radeon` (AMD), `nvidia-utils` (NVIDIA), or
`vulkan-intel` (Intel).

Debian / Ubuntu:

```sh
sudo apt install \
  build-essential cmake ninja-build clang pkg-config \
  libvulkan-dev vulkan-validationlayers-dev spirv-tools \
  libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev \
  libwayland-dev wayland-protocols libxkbcommon-dev
```

Note that Debian/Ubuntu CMake is usually far below 4.3 — install it from
[Kitware's APT repo](https://apt.kitware.com/) or the official binaries.

### Build

```sh
cmake --preset linux-debug-vulkan-tutorial
cmake --build --preset vulkan-tutorial
```

The binary lands at `build/linux-debug-vulkan-tutorial/VulkanTutorial`.

To build only the engine library, use the `linux-debug` preset instead — the
sample is gated behind `BUILD_VULKAN_TUTORIAL`.

## Windows

### Toolchain

The presets pin **MSVC** (`cl.exe`), x64. Install **Visual Studio 2022** (or the
standalone Build Tools) with the *Desktop development with C++* workload, which
provides `cl.exe`, the Windows SDK, CMake, and Ninja.

Configure from a **x64 Native Tools Command Prompt** so `cl.exe` is on `PATH`.
Visual Studio and VS Code with the CMake Tools extension handle this
automatically and will pick the presets up on their own.

### System packages

- **[Vulkan SDK from LunarG](https://vulkan.lunarg.com/sdk/home#windows)** —
  install it and let it set `VULKAN_SDK`; that is how `find_package(Vulkan)`
  finds the headers and loader. The validation layers ship with it.
- **CMake ≥ 4.3** — the one bundled with Visual Studio may lag behind; check
  `cmake --version` and install [from cmake.org](https://cmake.org/download/) if
  needed.

No X11/Wayland equivalent is needed — GLFW uses the Win32 backend.

### Build

```bat
cmake --preset x64-debug-vulkan-tutorial
cmake --build --preset vulkan-tutorial-windows
```

The binary lands at `build\x64-debug-vulkan-tutorial\VulkanTutorial.exe`.

## Presets

| Configure preset                | Platform | Notes                              |
| ------------------------------- | -------- | ---------------------------------- |
| `linux-debug`                   | Linux    | Engine only                        |
| `linux-debug-vulkan-tutorial`   | Linux    | Engine + sample                    |
| `x64-debug`                     | Windows  | Engine only                        |
| `x64-release`                   | Windows  | Engine only, Release               |
| `x64-debug-vulkan-tutorial`     | Windows  | Engine + sample                    |

| Build preset               | Configure preset               |
| -------------------------- | ------------------------------ |
| `vulkan-tutorial`          | `linux-debug-vulkan-tutorial`  |
| `vulkan-tutorial-windows`  | `x64-debug-vulkan-tutorial`    |

Configure presets are guarded by a host condition, so `cmake --list-presets`
only shows the ones for the OS you are on.

## Optional: Vulkan C++20 modules

Every preset sets `ENABLE_CPP19_MODULES=OFF`, which is the supported path. When
turned on, the build compiles `vulkan.cppm` from the Vulkan SDK as a named module
and the engine does `import vulkan_hpp` instead of including
`<vulkan/vulkan_raii.hpp>`.

```sh
cmake --preset linux-debug-vulkan-tutorial -DENABLE_CPP19_MODULES=ON
```

This needs a `vulkan.cppm` in the SDK include directory and a toolchain with
working C++20 module support. Treat it as experimental.

## Troubleshooting

**`CMakeCache.txt directory ... is different than the directory ...`** — a stale
`build/` from a previous location of the repo. Delete the offending directory
under `build/` and re-run the configure step.

**`Required layer not supported: VK_LAYER_KHRONOS_validation`** — the validation
layers are not installed. Install the package listed above, or build Release,
where they are compiled out.

**`Failed to find GPUs with Vulkan support!` / `failed to find a suitable GPU!`**
— no device reports Vulkan 1.3+ with `VK_KHR_swapchain`. Check your driver with
`vulkaninfo --summary`.

**Wayland or X11 headers not found** — install the corresponding `-dev` packages,
or disable that GLFW backend with `-DGLFW_BUILD_WAYLAND=OFF` /
`-DGLFW_BUILD_X11=OFF`.

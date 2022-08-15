# Aporia Blitz (build LD47)

[![build](https://github.com/dubgron/aporia-blitz/actions/workflows/build.yml/badge.svg)](https://github.com/dubgron/aporia-blitz/actions/workflows/build.yml)
[![emscripten](https://github.com/dubgron/aporia-blitz/actions/workflows/emscripten.yml/badge.svg)](https://github.com/dubgron/aporia-blitz/actions/workflows/emscripten.yml)

A **cross-platform**, **lightweight** yet **powerful 2D engine** to **make games in 48 hours** or less.

## Build Instructions

1. Clone repository
    ```sh
    git clone --recursive https://github.com/dubgron/aporia-blitz.git
    ```

2. Build with **CMake**
    ```sh
    # aporia-blitz
    mkdir build && cd build
    cmake -A x64 ..
    ```

3. Compile
    ```sh
    # aporia-blitz/build
    cmake --build .
    ```

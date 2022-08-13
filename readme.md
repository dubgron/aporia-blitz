# Aporia Blitz (build LD47)

[![Build Status](https://img.shields.io/travis/com/dubgron/aporia-blitz/master?token=3M7zid8xXNCxYNJKxB2L&label=master)](https://travis-ci.com/dubgron/aporia-blitz) 
[![Build Status](https://img.shields.io/travis/com/dubgron/aporia-blitz/develop?token=3M7zid8xXNCxYNJKxB2L&label=develop)](https://travis-ci.com/dubgron/aporia-blitz)

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

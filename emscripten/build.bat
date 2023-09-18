call ../../emsdk/emsdk_env.bat

call emcmake cmake .. -B build -DAPORIA_EMSCRIPTEN=ON "-DCMAKE_CXX_FLAGS=-std=c++20 -fbracket-depth=1000 -O2" "-DCMAKE_EXE_LINKER_FLAGS=--preload-file ../../../bin/content@content --shell-file ../../shell.html"

if not exist build mkdir build
emmake make -s -C build

# re3 Modern CMake Build

This project has been migrated to a modern CMake build system.

## Requirements
- CMake 3.16+
- Ninja Build System (Recommended)
- Clang Compiler (LLVM)

## How to Build (Windows)

### Default (OpenGL + SDL3)
Simply run:
```bat
build_win_clang.bat
```
Or manually:
```bash
mkdir build
cd build
cmake -G "Ninja" -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ ..
cmake --build .
```

### DirectX 9
Run:
```bat
build_win_clang_dx9.bat
```
Or manually:
```bash
mkdir build_dx9
cd build_dx9
cmake -G "Ninja" -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DREVC_USE_DX9=ON ..
cmake --build .
```

## IDE Support
You can open this folder directly in Visual Studio Code (with CMake Tools extension) or Visual Studio 2022.

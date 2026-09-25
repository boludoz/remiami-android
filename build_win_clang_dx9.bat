@echo off
if not exist build_dx9 mkdir build_dx9
cd build_dx9
cmake -G "Ninja" -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DREVC_USE_DX9=ON ..
cmake --build .
cd ..

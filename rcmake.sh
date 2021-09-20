mkdir -p build
cd build
cmake -G "Ninja"  -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake   -DCMAKE_BUILD_TYPE=Release   ../. 
cd ..
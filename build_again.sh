rm -rf build CMakeCache.txt
mkdir build && cd build
cmake -DCMAKE_PREFIX_PATH="/home/dhan/vcpkg/installed/x64-linux" ..
cmake --build .
cd build 
./2DPhysicsEngine


rm -rf build CMakeCache.txt
mkdir build && cd build
cmake -DCMAKE_PREFIX_PATH=<your vcpkg location>  ..
cmake --build .
cd build 
./2DPhysicsEngine


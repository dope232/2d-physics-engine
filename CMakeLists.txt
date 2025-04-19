# CMake configuration
cmake_minimum_required(VERSION 3.16)
project(2DPhysicsEngine)

set(CMAKE_CXX_STANDARD 17)

# Ensure CMake uses vcpkg
set(CMAKE_TOOLCHAIN_FILE "/home/dhan/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

# Manually set SFML_DIR
set(SFML_DIR "/home/dhan/vcpkg/installed/x64-linux/share/sfml")

# Find SFML
find_package(SFML 2.5 REQUIRED COMPONENTS graphics window system)
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(BZip2 REQUIRED)
find_package(Freetype REQUIRED)
find_package(unofficial-brotli CONFIG REQUIRED)


# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${PNG_LIBRARY_DIRS})


# Automatically detect all .cpp files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create executable
add_executable(2DPhysicsEngine ${SOURCES})

# Link SFML
target_link_libraries(2DPhysicsEngine 
    PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
    ${FREETYPE_LIBRARIES}
    ${PNG_LIBRARIES}  
    ${BZIP2_LIBRARIES}
    ${ZLIB_LIBRARIES}
    unofficial::brotli::brotlicommon
    unofficial::brotli::brotlidec
    unofficial::brotli::brotlienc
)
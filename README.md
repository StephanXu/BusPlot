

# Bus Plot

Cross-platform plotting for embedded devices status.

## Build

### 1. Install dependencies through Vcpkg

Generally, you could install dependencies through vcpkg easily. The example below has specified the platform (`x64-windows-static`), you can also choose other options to build for other platforms.

```bash
./vcpkg.exe install \
    --triplet x64-windows-static
    boost-asio \
    fmt \
    spdlog \
    glfw3 \
    glad \
    freetype
```

### 2. Build

You need to set `CMAKE_TOOLCHAIN_FILE` variable to `/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake`.

Build for debug: 

```bash
mkdir -p build/debug
cd build/debug
cmake ../.. -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Debug
cd ../..
```

Build for release:

```bash
mkdir -p build/release
cd build/release
cmake ../.. -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
cd ../..
```


# boost.http_io

## Visual Studio Solution

    cmake -G "Visual Studio 17 2022" -A win32 -B bin -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE="C:/Users/vinnie/src/boost/libs/http_io/cmake/toolchains/msvc.cmake"
    cmake -G "Visual Studio 17 2022" -A x64 -B bin64 -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE="C:/Users/vinnie/src/boost/libs/http_io/cmake/toolchains/msvc.cmake"

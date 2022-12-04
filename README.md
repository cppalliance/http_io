# boost.http_io

## Visual Studio Solution

    cmake -G "Visual Studio 16 2019" -A Win32 -B bin -DOPENSSL_ROOT_DIR=C:/Users/vinnie/vcpkg/installed/x86-windows -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/msvc.cmake
    cmake -G "Visual Studio 16 2019" -A x64 -B bin64 -DOPENSSL_ROOT_DIR=C:/Users/vinnie/vcpkg/installed/x64-windows -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/msvc.cmake

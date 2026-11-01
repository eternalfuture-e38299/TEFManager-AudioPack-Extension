cmake_minimum_required(VERSION 3.20)
project(tefkernel_cpp_wrapper CXX C)

set(CMAKE_CXX_STANDARD 17)

#[[
set(ANDROID_MODE OFF)
if (${ANDROID_MODE})
    set(ANDROID ON)
    add_compile_definitions(__ANDROID__)
    set(CMAKE_SYSROOT /home/eternalfuture/Android/Sdk/ndk/28.0.13004108/toolchains/llvm/prebuilt/linux-x86_64/sysroot/)
endif ()
]]

add_library(tefkernel_cpp_wrapper STATIC
        include/tefkernel/tef_api_imp.c
        src/memdl.cpp
        src/tefpkg.cpp
        src/patchlib/type.cpp
        src/patchlib/field.cpp
        src/patchlib/property.cpp
        src/patchlib/method.cpp
        src/patchlib/struct/string.cpp
)

target_include_directories(tefkernel_cpp_wrapper
        PUBLIC
        includes
        include
)
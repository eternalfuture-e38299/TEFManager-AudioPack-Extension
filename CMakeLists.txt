cmake_minimum_required(VERSION 3.20)
project(audiopack_extension)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(includes)

add_library(audiopack_extension SHARED
        includes/libs/miniz.c
        src/AudioPack.cpp
        src/AudioDecode.cpp
        src/SfxNameResolver.cpp
        src/AudioManager.cpp
        src/core.cpp
        src/XnbSound.cpp
)

add_subdirectory(tefkernel-cpp-wrapper)

target_link_libraries(audiopack_extension
        PRIVATE
        tefkernel_cpp_wrapper
)

if (ANDROID)
    target_link_libraries(audiopack_extension PRIVATE log)
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    if (CMAKE_SYSTEM_NAME STREQUAL "Linux" OR ANDROID OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        add_custom_command(TARGET audiopack_extension POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Stripping symbols from audiopack_extension..."
                COMMAND ${CMAKE_STRIP} --strip-all $<TARGET_FILE:audiopack_extension>
                COMMENT "Stripping symbols from audiopack_extension library using strip command"
                VERBATIM
        )
    endif()
endif ()


if(ANDROID)
    set(SYSTEM_NAME "android")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(SYSTEM_NAME "linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(SYSTEM_NAME "darwin")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(SYSTEM_NAME "windows")
else()
    set(SYSTEM_NAME "unknown")
endif()

# 获取架构

if(MINGW OR WIN32)
    # Windows: 通过编译器名称判断架构
    if(CMAKE_C_COMPILER MATCHES "x86_64-w64-mingw32" OR CMAKE_CXX_COMPILER MATCHES "x86_64-w64-mingw32")
        set(ARCH "x64")
    elseif(CMAKE_C_COMPILER MATCHES "i686-w64-mingw32" OR CMAKE_CXX_COMPILER MATCHES "i686-w64-mingw32")
        set(ARCH "x86")
    else()
        # 备用方案：通过指针大小判断
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(ARCH "x64")
        else()
            set(ARCH "x86")
        endif()
    endif()
else()
    # 非 Android 平台使用简化的架构名称
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64")
        set(ARCH "arm64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7|armv7a|ARMv7|arm")
        set(ARCH "arm")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64|x64")
        set(ARCH "x64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i686|i386|x86")
        set(ARCH "x86")
    else()
        set(ARCH "${CMAKE_SYSTEM_PROCESSOR}")
    endif()
endif()

#[[
message(STATUS "========== DEBUG ==========")
message(STATUS "SYSTEM_NAME: ${SYSTEM_NAME}")
message(STATUS "ARCH: ${ARCH}")
message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "WIN32: ${WIN32}")
message(STATUS "MINGW: ${MINGW}")
message(STATUS "============================")]]


# 设置输出文件名

set_target_properties(audiopack_extension PROPERTIES
        OUTPUT_NAME "module.${SYSTEM_NAME}.${ARCH}"
)
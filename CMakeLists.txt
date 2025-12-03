cmake_minimum_required(VERSION 3.15...3.31)
set(CMAKE_POLICY_VERSION_MINIMUM 3.15)

include(FetchContent)

project(hydra VERSION 0.2.3 LANGUAGES CXX)

if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(HYPERVISOR_ENABLED OFF)
    set(CUBEB_ENABLED OFF)
    set(FRONTEND "SwiftUI")
else ()
    option(HYPERVISOR_ENABLED "Enable the Hypervisor CPU backend" ON)
    option(CUBEB_ENABLED "Enable the cubeb audio backend" ON)
    set(FRONTEND "SDL3" CACHE STRING "Choose which frontend to use.")
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    option(MACOS_BUNDLE "Build a macOS app bundle" OFF)
elseif (CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(MACOS_BUNDLE ON)
else ()
    set(MACOS_BUNDLE OFF)
endif ()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(GIT_HASH_SHORT "")
else ()
    # Set up Git hash (only if git is available and we're in a git repo)
    execute_process(
        COMMAND git rev-parse HEAD
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    # Get the first 7 characters
    string(SUBSTRING ${GIT_HASH} 0 7 GIT_HASH_SHORT)
    if(NOT GIT_HASH_SHORT)
        set(GIT_HASH_SHORT "0000000")
    endif()

    if(GIT_DIRTY)
        set(GIT_HASH_SHORT "${GIT_HASH_SHORT}+dirty")
    endif()
endif ()

if (NOT CMAKE_BUILD_TYPE)
    message("Build Type not set, defaulting to Debug...")
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Enable Objective-C and Swift on Apple platforms
if (APPLE)
    enable_language(OBJC OBJCXX Swift)
endif()

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# HACK: link C++ standard library
if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
    link_libraries(c++)
endif ()

# Packages
find_package(fmt CONFIG)
if (NOT fmt_FOUND)
    message("fmt not found, downloading from GitHub")

    FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG 10.2.1
    )
    FetchContent_MakeAvailable(fmt)
endif ()

# Externals

# toml11
#add_subdirectory(externals/toml11) # Hatch already uses toml11

# dynarmic
add_subdirectory(externals/dynarmic)

# cubeb
if (CUBEB_ENABLED)
    add_subdirectory(externals/cubeb)
endif ()

# luft
#add_subdirectory(externals/luft)

# hatch
add_subdirectory(externals/hatch)

# libyaz0
add_subdirectory(externals/libyaz0)

# nx2elf
add_subdirectory(externals/nx2elf)

set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

include_directories(externals/metal-cpp externals/dynarmic externals/stb)

if (HYPERVISOR_ENABLED)
    add_compile_definitions(HYDRA_HYPERVISOR_ENABLED=1)
else ()
    add_compile_definitions(HYDRA_HYPERVISOR_ENABLED=0)
endif ()

if (CUBEB_ENABLED)
    add_compile_definitions(HYDRA_CUBEB_ENABLED=1)
else ()
    add_compile_definitions(HYDRA_CUBEB_ENABLED=0)
endif ()

if (APPLE)
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC NO)
endif ()

# TODO: find out why release builds are broken
# TODO: do compile flags need to be specified manually?
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g")
    add_definitions(-DHYDRA_DEBUG)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
elseif (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g")
elseif (CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")
else ()
    message(FATAL_ERROR "Unknown build type: ${CMAKE_BUILD_TYPE}")
endif ()

add_subdirectory(src)

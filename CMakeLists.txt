cmake_minimum_required(VERSION 3.15...3.31)

option(CODESIGN_IDENTITY "The identity to use for macOS code signing.")

# TODO: use c++20
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(hydra VERSION 1.0 LANGUAGES CXX)

if (APPLE)
    enable_language(OBJC OBJCXX)
endif()

find_package(fmt CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

#add_subdirectory(externals/fmt)

#include_directories(externals/fmt/include)
include_directories(externals/metal-cpp)

add_subdirectory(src)

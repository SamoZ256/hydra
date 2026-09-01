#pragma once

// Compiler

#if defined(__clang__)
#define ZTD_COMPILER_CLANG
#elifdef __GNUC__
#define ZTD_COMPILER_GCC
#elifdef __INTEL_COMPILER
#define ZTD_COMPILER_INTEL_CPP
#elifdef _MSC_VER
#define ZTD_COMPILER_MSVC
#elifdef _MSC_VER
#define ZTD_COMPILER_MSVC
#elif defined(__MINGW32__) || defined(__MINGW64__)
#define ZTD_COMPILER_MINGW
#else
#define ZTD_COMPILER_UNKNOWN
#endif

// Architecture

#if defined(__aarch64__) || defined(_M_ARM64)
#define ZTD_ARCH_AARCH64
#elif defined(__arm__) || defined(_M_ARM)
#define ZTD_ARCH_ARM32
#elif defined(__x86_64__) || defined(_M_X64)
#define ZTD_ARCH_X86_64
#elifdef __riscv
#define ZTD_ARCH_RISCV
#else
#define ZTD_ARCH_UNKNOWN
#endif

// Platform

// Windows
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) ||                \
    defined(__WINDOWS__)
#define ZTD_PLATFORM_WINDOWS
#ifdef _WIN64
#define ZTD_PLATFORM_WINDOWS64
#else
#define ZTD_PLATFORM_WINDOWS32
#endif

// Apple platforms
#elif defined(__APPLE__) || defined(__MACH__)
#include <TargetConditionals.h>
#define ZTD_PLATFORM_APPLE
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#define ZTD_PLATFORM_IOS
#elif TARGET_OS_MAC
#define ZTD_PLATFORM_MACOS
#endif

// Android
#elifdef __ANDROID__
#define ZTD_PLATFORM_ANDROID

// Linux
#elif defined(__linux__) || defined(__linux)
#define ZTD_PLATFORM_LINUX

// FreeBSD
#elifdef __FreeBSD__
#define ZTD_PLATFORM_FREEBSD

#else
#error ZTD_PLATFORM_UNKNOWN
#endif

// Unix
#if defined(__unix__) || defined(__unix)
#define ZTD_PLATFORM_UNIX
#endif

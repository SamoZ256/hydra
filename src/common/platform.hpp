#pragma once

// Windows
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) ||                \
    defined(__WINDOWS__)
#define PLATFORM_WINDOWS 1
#ifdef _WIN64
#define PLATFORM_WINDOWS_64 1
#else
#define PLATFORM_WINDOWS_32 1
#endif

// Apple platforms
#elif defined(__APPLE__) || defined(__MACH__)
#include <TargetConditionals.h>
#define PLATFORM_APPLE
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#define PLATFORM_IOS 1
#elif TARGET_OS_MAC
#define PLATFORM_MACOS 1
#endif

// Android
#elif defined(__ANDROID__)
#define PLATFORM_ANDROID 1

// Linux
#elif defined(__linux__) || defined(__linux)
#define PLATFORM_LINUX 1

// FreeBSD
#elif defined(__FreeBSD__)
#define PLATFORM_FREEBSD 1

// Generic Unix
#elif defined(__unix__) || defined(__unix)
#define PLATFORM_UNIX 1

#else
#error "Unknown platform"
#endif

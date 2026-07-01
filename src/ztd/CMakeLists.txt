cmake_minimum_required(VERSION 3.15...3.31)
set(CMAKE_POLICY_VERSION_MINIMUM 3.15)

project(ztd VERSION 0.0.1 LANGUAGES CXX)

option(ZTD_PRECOMPILE_HEADERS "Precompile the main ztd header" OFF)
option(ZTD_CLANG_TIDY_ENABLED "Enable clang-tidy" ON)
option(ZTD_FMT_ENABLED "Enable fmt support for some types" OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#set(CMAKE_CXX_MODULE_STD ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

# Enable Objective-C on Apple platforms
if (APPLE)
    enable_language(OBJC OBJCXX)
endif()

if (ZTD_CLANG_TIDY_ENABLED)
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-use-color;-extra-arg-before=-Wno-unknown-warning-option")
endif ()

add_library(ztd
    src/ztd/compress/lz4.cpp
    src/ztd/compress/lz4.hpp
    src/ztd/hash/xxhash32.hpp
    src/ztd/macros/constructor_helper.hpp
    src/ztd/macros/crtp_helper.hpp
    src/ztd/macros/enum_helper.hpp
    src/ztd/macros/for_each_helper.hpp
    src/ztd/macros/macro_helper.hpp
    src/ztd/macros/optional_helper.hpp
    src/ztd/macros/platform.hpp
    src/ztd/mem/alignment.hpp
    src/ztd/mem/allocator.hpp
    src/ztd/mem/c_allocator.hpp
    src/ztd/mem/default_allocator.hpp
    src/ztd/mem/literals.hpp
    src/ztd/mem/page.hpp
    src/ztd/mem/page_allocator.hpp
    src/ztd/mem/static_pool.hpp
    src/ztd/linked_list.hpp
    src/ztd/range.hpp
    src/ztd/time.hpp
    src/ztd/type_aliases.hpp
    src/ztd/ztd.hpp
)

set_target_properties(ztd PROPERTIES CXX_SCAN_FOR_MODULES ON)
target_compile_features(ztd PUBLIC cxx_std_23)

target_compile_options(ztd PRIVATE
    # TODO: uncomment
    #-fno-exceptions
    -fno-asynchronous-unwind-tables
    -fno-rtti
    -fno-threadsafe-statics
    -fvisibility=hidden

    # Warnings
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wsign-conversion
    -Wimplicit-int-float-conversion
    -Wshadow
    -Wdouble-promotion
    -Wundef
    -Wnon-virtual-dtor
    -Wold-style-cast

    # Disabled warnings
    -Wno-missing-designated-field-initializers

    # Extensions
    -Wno-c99-extensions
    -Wno-gnu-anonymous-struct
    -Wno-nested-anon-types
    -Wno-zero-length-array
    -Wno-vla-extension
    -Wno-gnu-case-range
    -Wno-gnu-zero-variadic-macro-arguments
)

target_include_directories(ztd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

if (ZTD_PRECOMPILE_HEADERS)
    target_precompile_headers(ztd PUBLIC src/ztd/ztd.hpp)
endif ()

add_library(ztd::ztd ALIAS ztd)

include_directories(.)

if (CLANG_TIDY_ENABLED)
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-use-color;-extra-arg-before=-Wno-unknown-warning-option")
endif ()

add_subdirectory(common)
add_subdirectory(core)
add_subdirectory(frontend)

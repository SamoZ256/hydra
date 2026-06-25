include_directories(.)

set(ZTD_PRECOMPILE_HEADERS ON)
set(ZTD_CLANG_TIDY_ENABLED ${CLANG_TIDY_ENABLED})
set(ZTD_FMT_ENABLED ON)
add_subdirectory(ztd)

if (CLANG_TIDY_ENABLED)
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-use-color;-extra-arg-before=-Wno-unknown-warning-option")
endif ()

add_subdirectory(common)
add_subdirectory(core)
add_subdirectory(frontend)

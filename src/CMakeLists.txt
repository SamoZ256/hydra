include_directories(.)

set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-extra-arg-before=-Wno-unknown-warning-option;-use-color")

add_subdirectory(common)
add_subdirectory(core)
add_subdirectory(frontend)

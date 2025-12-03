add_library(hydra-common
    platform.hpp
    macros.hpp
    type_aliases.hpp
    types.hpp
    traits.hpp
    functions.hpp
    atomic.hpp
    literals.hpp
    string.hpp
    time.hpp
    memory.hpp
    linked_list.hpp
    pool.hpp
    static_pool.hpp
    dynamic_pool.hpp
    filesystem.hpp
    log.cpp
    log.hpp
    lz4.cpp
    lz4.hpp
    toml_helper.hpp
    config.cpp
    config.hpp
    common.hpp
    elf.h
    stb.cpp
)

target_link_libraries(hydra-common PUBLIC fmt::fmt)
target_link_libraries(hydra-common PUBLIC toml11::toml11)

target_precompile_headers(hydra-common PUBLIC common.hpp)

# TODO: make a precompiled header
add_library(hydra-common
    macros.hpp
    type_aliases.hpp
    types.hpp
    traits.hpp
    functions.hpp
    literals.hpp
    string.hpp
    time.hpp
    memory.hpp
    static_pool.hpp
    dynamic_pool.hpp
    log.cpp
    log.hpp
    lz4.cpp
    lz4.hpp
    config.cpp
    config.hpp
    common.hpp
)
target_precompile_headers(hydra-common PUBLIC common.hpp)

target_link_libraries(hydra-common PUBLIC fmt::fmt)
target_link_libraries(hydra-common PRIVATE toml11::toml11)

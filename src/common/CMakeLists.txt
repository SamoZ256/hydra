add_library(hydra-common
    platform.hpp
    macros.hpp
    type_aliases.hpp
    types.hpp
    range.hpp
    traits.hpp
    functions.hpp
    atomic.hpp
    literals.hpp
    optional_helper.hpp
    string.hpp
    time.hpp
    hash.hpp
    linked_list.hpp
    pool.hpp
    static_pool.hpp
    dynamic_pool.hpp
    small_cache.hpp
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
    io/stream.hpp
    io/continuous_stream.hpp
    io/memory_stream.hpp
    io/iostream_stream.hpp
    io/stream_view.hpp
    io/sparse_stream.hpp
)

target_link_libraries(hydra-common PRIVATE hydra_warnings)
target_link_libraries(hydra-common PUBLIC fmt::fmt toml11::toml11)

target_precompile_headers(hydra-common PUBLIC common.hpp)

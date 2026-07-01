add_library(hydra-common
    macros.hpp
    type_aliases.hpp
    handle.hpp
    types.hpp
    traits.hpp
    functions.hpp
    atomic.hpp
    string.hpp
    small_cache.hpp
    filesystem.hpp
    log.cpp
    log.hpp
    toml_helper.hpp
    fmt_helper.hpp
    config.cpp
    config.hpp
    io/stream.hpp
    io/continuous_stream.hpp
    io/memory_stream.hpp
    io/iostream_stream.hpp
    io/stream_view.hpp
    io/sparse_stream.hpp
)

target_precompile_headers(hydra-common PUBLIC common.hpp)

target_link_libraries(hydra-common PRIVATE hydra_compile_options)
target_link_libraries(hydra-common PUBLIC fmt::fmt toml11::toml11 ztd::ztd)

if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set_target_properties(hydra-common PROPERTIES
        XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET ${IOS_DEPLOYMENT_TARGET}
    )
endif ()

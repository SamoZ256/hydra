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
)

target_precompile_headers(hydra-common PUBLIC common.hpp)

find_package(Python3 COMPONENTS Interpreter QUIET)
if (HYDRA_LINT_TARGET AND Python3_FOUND AND ZTD_LINT_SCRIPT)
    add_custom_target(hydra-lint
        COMMAND ${Python3_EXECUTABLE} ${ZTD_LINT_SCRIPT}
                ${CMAKE_BINARY_DIR} ${HYDRA_LINT_EXTRA_BUILD_DIRS}
                --sources "${CMAKE_SOURCE_DIR}/src/.*\\.(cpp|mm)"
                --header-filter "${CMAKE_SOURCE_DIR}/src/.*"
                --changed
        USES_TERMINAL
        VERBATIM
    )
endif ()

target_link_libraries(hydra-common PRIVATE hydra_compile_options)
target_link_libraries(hydra-common PUBLIC fmt::fmt toml11::toml11 ztd::ztd)

if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set_target_properties(hydra-common PROPERTIES
        XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET ${IOS_DEPLOYMENT_TARGET}
    )
endif ()

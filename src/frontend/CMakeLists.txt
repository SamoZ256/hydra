if (FRONTEND STREQUAL "SDL3")
    message(STATUS "Using SDL3 frontend")

    set(SOURCE_FILES
        sdl3/main.cpp
        sdl3/const.hpp
        sdl3/window.cpp
        sdl3/window.hpp
    )

    find_package(SDL3 CONFIG REQUIRED)
    set(LINK_LIBS SDL3::SDL3)
elseif (FRONTEND STREQUAL "SwiftUI")
    message(STATUS "Using SwiftUI frontend")

    set(SOURCE_FILES
        swiftui/HydraApp.swift
        swiftui/ContentView.swift
        swiftui/MenuCommands.swift
    )
else ()
    message(FATAL_ERROR "Unknown frontend \"${FRONTEND}\". Valid options are \"SDL3\" and \"SwiftUI\".")
endif ()

add_executable(hydra ${SOURCE_FILES})

target_link_libraries(hydra PRIVATE hydra-common hydra-core ${LINK_LIBS})

# Signing
if (NOT CODESIGN_IDENTITY)
    message(FATAL_ERROR "CODESIGN_IDENTITY not set. Please set it to the identity you want to use for code signing.")
endif ()

set(ENTITLEMENTS_FILE "${PROJECT_SOURCE_DIR}/hydra.entitlements")

add_custom_command(TARGET hydra POST_BUILD VERBATIM
    COMMAND codesign --force --sign "${CODESIGN_IDENTITY}" --entitlements ${ENTITLEMENTS_FILE} --timestamp --options runtime "$<TARGET_FILE:hydra>"
    COMMENT "Signing hydra with entitlements"
)

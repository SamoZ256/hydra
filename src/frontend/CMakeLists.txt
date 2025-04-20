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
        swiftui/ToolbarItems.swift
        swiftui/Settings/GeneralSettingsView.swift
        swiftui/Settings/GraphicsSettingsView.swift
        swiftui/Settings/DebugSettingsView.swift
        swiftui/Settings/SettingsView.swift
        swiftui/Sidebar/DetailView.swift
        swiftui/Sidebar/GamelistView.swift
        swiftui/Sidebar/NowPlayingView.swift
        swiftui/Sidebar/SidebarSelectionState.swift
        swiftui/Sidebar/SidebarView.swift
    )
else ()
    message(FATAL_ERROR "Unknown frontend \"${FRONTEND}\". Valid options are \"SDL3\" and \"SwiftUI\".")
endif ()

add_executable(hydra MACOSX_BUNDLE ${SOURCE_FILES})

if (FRONTEND STREQUAL "SwiftUI")
    set(RESOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/hydra.icns)
    target_sources(hydra PRIVATE "${RESOURCE_FILES}")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        RESOURCE "${RESOURCE_FILES}"
        MACOSX_BUNDLE_GUI_IDENTIFIER com.github.${PROJECT_NAME}
        MACOSX_BUNDLE_ICON_FILE "hydra.icns"
        MACOSX_BUNDLE_INFO_STRING "Hydra"
        MACOSX_BUNDLE_BUNDLE_VERSION "${BUILD_HASH}"
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
    )
    set(MACOSX_MINIMUM_SYSTEM_VERSION "12.0")
    set(MACOSX_BUNDLE_CATEGORY "public.app-category.games")
    set(MACOSX_BUNDLE_BUNDLE_NAME "Hydra")
    set(MACOSX_BUNDLE_COPYRIGHT "Copyright© Hydra Project")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${EMULATOR_VERSION_MAJOR}.${EMULATOR_VERSION_MINOR}.${EMULATOR_VERSION_PATCH}")
    
endif()

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

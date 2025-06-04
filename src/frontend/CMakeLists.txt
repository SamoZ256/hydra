# TODO: set based on platform
set(NATIVE_SOURCE_FILES
    native/cocoa/native.hpp
    native/cocoa/native.mm
)

if (FRONTEND STREQUAL "SDL3")
    message(STATUS "Using SDL3 frontend")
    set(CMAKE_OSX_DEPLOYMENT_TARGET 13.0)

    find_package(SDL3 CONFIG REQUIRED)
    set(LINK_LIBS SDL3::SDL3)

    add_executable(hydra ${NATIVE_SOURCE_FILES}
        sdl3/main.cpp
        sdl3/const.hpp
        sdl3/cursor.cpp
        sdl3/cursor.hpp
        sdl3/window.cpp
        sdl3/window.hpp
    )
elseif (FRONTEND STREQUAL "SwiftUI")
    message(STATUS "Using SwiftUI frontend")
    set(CMAKE_OSX_DEPLOYMENT_TARGET 15.0)

    add_executable(hydra
        swiftui/HydraApp.swift
        swiftui/ContentView.swift
        swiftui/MenuCommands.swift
        swiftui/ToolbarItems.swift
        swiftui/Game.swift
        swiftui/MetalView.swift
        swiftui/EmulationView.swift
        swiftui/GamePreview.swift
        swiftui/GameListView.swift
        swiftui/helpers/StringList.swift
        swiftui/helpers/EditableStringList.swift
        swiftui/helpers/EditablePathList.swift
        swiftui/settings/GeneralSettingsView.swift
        swiftui/settings/CpuSettingsView.swift
        swiftui/settings/GraphicsSettingsView.swift
        swiftui/settings/DebugSettingsView.swift
        swiftui/settings/SettingsView.swift
    )

    target_compile_options(hydra PRIVATE -import-objc-header ${CMAKE_SOURCE_DIR}/src/frontend/swiftui/bridging_header.h)
else ()
    message(FATAL_ERROR "Unknown frontend \"${FRONTEND}\". Valid options are \"SDL3\" and \"SwiftUI\".")
endif ()

target_link_libraries(hydra PRIVATE hydra-common hydra-core "-framework Cocoa" ${LINK_LIBS})

if (MACOS_BUNDLE)
    set(RESOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/hydra.icns)
    target_sources(hydra PRIVATE ${RESOURCE_FILES})
    set_target_properties(${PROJECT_NAME} PROPERTIES
        RESOURCE ${RESOURCE_FILES}
        OUTPUT_NAME "Hydra"
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.github.${PROJECT_NAME}"
        MACOSX_BUNDLE_ICON_FILE "hydra.icns"
        #MACOSX_BUNDLE_BUNDLE_VERSION ${BUILD_HASH}
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
    )
    if (FRONTEND STREQUAL "SwiftUI")
        set(MACOSX_MINIMUM_SYSTEM_VERSION "15.0")
    else ()
        set(MACOSX_MINIMUM_SYSTEM_VERSION "13.0")
    endif ()
    set(MACOSX_BUNDLE_CATEGORY "public.app-category.games")
    set(MACOSX_BUNDLE_BUNDLE_NAME "Hydra")
    set(MACOSX_BUNDLE_COPYRIGHT "Copyright© Hydra Project")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH}")
endif ()

# Signing
set(ENTITLEMENTS_FILE "${PROJECT_SOURCE_DIR}/hydra.entitlements")

# TODO: use --deep?
add_custom_command(TARGET hydra POST_BUILD VERBATIM
    COMMAND codesign --entitlements ${ENTITLEMENTS_FILE} --force --sign - --timestamp --options runtime "$<TARGET_FILE:hydra>"
    COMMENT "Signing hydra with entitlements"
)

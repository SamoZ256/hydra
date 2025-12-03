# TODO: set based on platform
set(NATIVE_SOURCE_FILES
    native/cocoa/native.hpp
    native/cocoa/native.mm
)

if (FRONTEND STREQUAL "SDL3")
    message(STATUS "Using SDL3 frontend")
    set(CMAKE_OSX_DEPLOYMENT_TARGET 13.0)
    set(MACOSX_MINIMUM_SYSTEM_VERSION "13.0")

    find_package(SDL3 CONFIG REQUIRED)

    add_executable(hydra ${NATIVE_SOURCE_FILES}
        sdl3/main.cpp
        sdl3/const.hpp
        sdl3/cursor.cpp
        sdl3/cursor.hpp
        sdl3/window.cpp
        sdl3/window.hpp
    )

    target_link_libraries(hydra PRIVATE SDL3::SDL3 "-framework Cocoa")
elseif (FRONTEND STREQUAL "SwiftUI")
    message(STATUS "Using SwiftUI frontend")
    set(CMAKE_OSX_DEPLOYMENT_TARGET 15.0)
    set(MACOSX_MINIMUM_SYSTEM_VERSION "15.0")
    set(SWIFT_BUILD_TYPE_CONDITION "$<UPPER_CASE:$<CONFIG>>")

    add_executable(hydra
        swiftui/Api.swift
        swiftui/HydraApp.swift
        swiftui/ContentView.swift
        swiftui/MenuCommands.swift
        swiftui/Game.swift
        swiftui/MetalView.swift
        swiftui/EmulationView.swift
        swiftui/GamePreview.swift
        swiftui/GameListView.swift
        swiftui/helpers/Error.swift
        swiftui/helpers/Math.swift
        swiftui/helpers/UrlResolver.swift
        swiftui/helpers/StringList.swift
        swiftui/helpers/EditableStringList.swift
        swiftui/helpers/EditablePathList.swift
        swiftui/helpers/ClickableListItem.swift
        swiftui/helpers/GameLoader.swift
        swiftui/settings/GeneralSettingsView.swift
        swiftui/settings/CpuSettingsView.swift
        swiftui/settings/GraphicsSettingsView.swift
        swiftui/settings/AudioSettingsView.swift
        swiftui/settings/UserAvatarView.swift
        swiftui/settings/UserPreview.swift
        swiftui/settings/UserAvatarPickerView.swift
        swiftui/settings/UserAvatarEditorView.swift
        swiftui/settings/UserEditorView.swift
        swiftui/settings/UserSettingsView.swift
        swiftui/settings/SystemSettingsView.swift
        swiftui/settings/DebugSettingsView.swift
        swiftui/settings/SettingsView.swift
        swiftui/toolbars/GameListToolbarItems.swift
        swiftui/toolbars/InGameToolbarItems.swift
        swiftui/debugger/Message.swift
        swiftui/debugger/MessageView.swift
        swiftui/debugger/ThreadDebuggerView.swift
        swiftui/debugger/DebuggerView.swift
        swiftui/debugger/DebuggerListView.swift
        swiftui/debugger/DebuggersView.swift
    )

    if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        target_sources(hydra PRIVATE swiftui/macos/AppDelegate.swift)
    endif ()

    target_compile_definitions(hydra PRIVATE ${SWIFT_BUILD_TYPE_CONDITION})
    target_compile_options(hydra PRIVATE -import-objc-header ${CMAKE_SOURCE_DIR}/src/core/c_api.h)
else ()
    message(FATAL_ERROR "Unknown frontend \"${FRONTEND}\". Valid options are \"SDL3\" and \"SwiftUI\".")
endif ()

target_link_libraries(hydra PRIVATE hydra-common hydra-core)

if (MACOS_BUNDLE)
    set(RESOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/hydra.icns ${CMAKE_SOURCE_DIR}/res/nx-hbloader/hbl.nsp ${CMAKE_SOURCE_DIR}/res/images/default_avatar_image.png ${CMAKE_SOURCE_DIR}/res/lldb/.lldbinit ${CMAKE_SOURCE_DIR}/res/lldb/commands.py)
    target_sources(hydra PRIVATE ${RESOURCE_FILES})
    set_target_properties(hydra PROPERTIES
        RESOURCE "${RESOURCE_FILES}"
        OUTPUT_NAME "Hydra"
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.github.${PROJECT_NAME}"
        MACOSX_BUNDLE_ICON_FILE "hydra.icns"
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
    )

    if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
        set_target_properties(hydra PROPERTIES
            XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED YES
            XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED YES
            XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Automatic"
            XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.github.${PROJECT_NAME}"
            XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 17.4.0
            XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"  # iPhone and iPad
            # TODO: development team
            XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${PROJECT_SOURCE_DIR}/hydra.ios.entitlements"
        )
    endif ()

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set_target_properties(hydra PROPERTIES
            MACOSX_BUNDLE_BUNDLE_VERSION ${GIT_HASH_SHORT}
        )
    endif ()

    set(MACOSX_BUNDLE_CATEGORY "public.app-category.games")
    set(MACOSX_BUNDLE_BUNDLE_NAME "Hydra")
    set(MACOSX_BUNDLE_COPYRIGHT "Copyright© Hydra Project")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH}")
endif ()

# Signing
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(ENTITLEMENTS_FILE "${PROJECT_SOURCE_DIR}/hydra.macos.entitlements")

    # TODO: use --deep?
    add_custom_command(TARGET hydra POST_BUILD VERBATIM
        COMMAND codesign --entitlements ${ENTITLEMENTS_FILE} --force --sign - --timestamp --options runtime "$<TARGET_FILE:hydra>"
        COMMENT "Signing hydra with entitlements"
    )
endif ()

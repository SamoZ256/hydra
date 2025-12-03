import SwiftUI

@main
struct HydraApp: App {
    #if os(macOS)
        @NSApplicationDelegateAdaptor(AppDelegate.self) var appDelegate
    #endif

    @State private var activeGame: Game? = nil
    @State private var emulationContext: HydraEmulationContext? = nil

    var body: some Scene {
        #if os(macOS)
            Window("Hydra", id: "main") {
                ContentView(activeGame: self.$activeGame, emulationContext: self.$emulationContext)
            }
            .defaultSize(width: 1280, height: 720)
            .windowResizability(.contentSize)
            .commands {
                MenuCommands(activeGame: self.$activeGame, emulationContext: self.$emulationContext)
            }

            Window("Debugger", id: "debugger") {
                DebuggersView()
            }
            .defaultLaunchBehavior(.suppressed)

            Settings {
                SettingsView()
            }
        #else
            WindowGroup {
                ContentView(activeGame: self.$activeGame, emulationContext: self.$emulationContext)
            }
        #endif
    }
}

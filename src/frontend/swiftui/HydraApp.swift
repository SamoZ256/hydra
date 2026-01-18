import SwiftUI

class GlobalState: ObservableObject {
    // Game library
    @Published var gamePaths: [String] = []
    @Published var loaderPluginChangeID = 0  // HACK

    // Emulation
    @Published var activeGame: Game? = nil
    @Published var emulationContext: HydraEmulationContext? = nil
    @Published var isStopping = false
    @Published var isHandheldMode = hydraConfigGetHandheldMode().pointee

    init() {
        hydraLoaderPluginManagerRefresh()

        let gamePathsOption = hydraConfigGetGamePaths()
        for i in 0..<gamePathsOption.count {
            let gamePath = gamePathsOption.get(at: i)
            gamePaths.append(gamePath)
        }
    }

    func refreshLoaderPluginManager() {
        hydraLoaderPluginManagerRefresh()
        loaderPluginChangeID += 1
    }
}

@main
struct HydraApp: App {
    #if os(macOS)
        @NSApplicationDelegateAdaptor(AppDelegate.self) var appDelegate
    #endif

    @State private var globalState = GlobalState()

    var body: some Scene {
        #if os(macOS)
            Window("Hydra", id: "main") {
                ContentView()
                    .environmentObject(globalState)
            }
            .defaultSize(width: 1280, height: 720)
            .windowResizability(.contentSize)
            .commands {
                MenuCommands(globalState: $globalState)
            }

            Window("Debugger", id: "debugger") {
                DebuggersView()
            }
            .defaultLaunchBehavior(.suppressed)

            Settings {
                SettingsView()
                    .environmentObject(globalState)
            }
        #else
            WindowGroup {
                ContentView()
                    .environmentObject(globalState)
            }
        #endif
    }
}

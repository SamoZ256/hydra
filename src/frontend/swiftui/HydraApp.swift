import SwiftUI

class GlobalState: ObservableObject {
    // Game library
    @Published var gamePaths: [String] = []
    let loaderPluginManager = HydraLoaderPluginManager()
    @Published var loaderPluginRefreshID = 0

    // Emulation
    @Published var activeGame: Game? = nil
    @Published var system: HydraSystem? = nil
    @Published var isStopping = false
    @Published var isHandheldMode: Bool {
        didSet {
            hydraConfigGetHandheldMode().pointee = isHandheldMode
            hydraConfigSerialize()
            guard let system = system else { return }
            system.notifyOperationModeChanged()
        }
    }

    init() {
        isHandheldMode = hydraConfigGetHandheldMode().pointee

        let gamePathsOption = hydraConfigGetGamePaths()
        for i in 0..<gamePathsOption.count {
            let gamePath = gamePathsOption.get(at: i)
            gamePaths.append(gamePath)
        }
    }

    func refreshLoaderPluginManager() {
        loaderPluginManager.refresh()
        loaderPluginRefreshID += 1
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

            Window("Texture Viewer", id: "texture-viewer") {
                TextureViewer()
                    .environmentObject(globalState)
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

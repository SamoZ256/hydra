import SwiftUI

#if os(iOS)
    import GameController
#endif

struct ContentView: View {
    @EnvironmentObject var globalState: GlobalState

    @AppStorage("viewMode") private var viewMode: ViewMode = .list
    @State private var fps: Int = 0

    #if os(iOS)
        @State private var virtualController: GCVirtualController? = nil
    #endif

    private var appVersion: String {
        Bundle.main.infoDictionary?["CFBundleShortVersionString"] as? String ?? "?"
    }

    private var gitVersion: String {
        Bundle.main.infoDictionary?["CFBundleVersion"] as? String ?? "?"
    }

    private var navigationTitle: String {
        let gameTitle = globalState.activeGame?.name ?? ""
        var titleInsert = ""
        if globalState.activeGame?.name != nil { titleInsert = "  |  \(gameTitle)" }
        var gitInsert = ""
        if gitVersion != "" { gitInsert = " (\(gitVersion))" }
        var fpsInsert = ""
        if globalState.system != nil { fpsInsert = "  |  \(fps) FPS" }
        return "Hydra v\(appVersion)\(gitInsert)\(titleInsert)\(fpsInsert)"
    }

    var body: some View {
        NavigationStack {
            GameListView(viewMode: $viewMode)
                .navigationDestination(item: $globalState.activeGame) { activeGame in
                    EmulationView(fps: $fps)
                        .aspectRatio(CGSize(width: 16, height: 9), contentMode: .fit)
                        #if os(iOS)
                            .onAppear {
                                // TODO: if virtual controller enabled
                                if true {
                                    let config = GCVirtualController.Configuration()
                                    config.elements = [
                                        GCInputButtonA, GCInputButtonB, GCInputButtonX,
                                        GCInputButtonY,
                                        GCInputLeftThumbstick, GCInputRightThumbstick,
                                        GCInputLeftShoulder,
                                        GCInputRightShoulder, GCInputLeftTrigger,
                                        GCInputRightTrigger,
                                    ]

                                    let virtualController = GCVirtualController(
                                        configuration: config)
                                    virtualController.connect()

                                    self.virtualController = virtualController
                                }
                            }
                            .onDisappear {
                                virtualController?.disconnect()
                                virtualController = nil
                            }
                        #endif
                        .toolbar {
                            EmulationToolbarItems()
                        }
                }
                .toolbar {
                    GameListToolbarItems(viewMode: $viewMode)
                }
                .navigationBarBackButtonHidden()
        }
        #if os(macOS)
            .windowToolbarFullScreenVisibility(.onHover)
        #endif
        .navigationTitle(navigationTitle)
    }
}

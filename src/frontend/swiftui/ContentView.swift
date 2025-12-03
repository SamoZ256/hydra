import SwiftUI

#if os(iOS)
    import GameController
#endif

struct ContentView: View {
    @Binding var activeGame: Game?
    @Binding var emulationContext: HydraEmulationContext?

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
        let gameTitle = activeGame?.name ?? ""
        var titleInsert = ""
        if activeGame?.name != nil { titleInsert = "  |  \(gameTitle)" }
        var gitInsert = ""
        if gitVersion != "" { gitInsert = " (\(gitVersion))" }
        var fpsInsert = ""
        if emulationContext != nil { fpsInsert = "  |  \(fps) FPS" }
        return "Hydra v\(appVersion)\(gitInsert)\(titleInsert)\(fpsInsert)"
    }

    var body: some View {
        NavigationStack {
            GameListView(activeGame: self.$activeGame)
                .navigationDestination(item: self.$activeGame) { activeGame in
                    EmulationView(
                        game: activeGame, emulationContext: self.$emulationContext, fps: $fps
                    )
                    .aspectRatio(CGSize(width: 16, height: 9), contentMode: .fit)
                    #if os(iOS)
                        .onAppear {
                            // TODO: if virtual controller enabled
                            if true {
                                let config = GCVirtualController.Configuration()
                                config.elements = [
                                    GCInputButtonA, GCInputButtonB, GCInputButtonX, GCInputButtonY,
                                    GCInputLeftThumbstick, GCInputRightThumbstick,
                                    GCInputLeftShoulder,
                                    GCInputRightShoulder, GCInputLeftTrigger, GCInputRightTrigger,
                                ]

                                let virtualController = GCVirtualController(configuration: config)
                                virtualController.connect()

                                self.virtualController = virtualController
                            }
                        }
                        .onDisappear {
                            virtualController?.disconnect()
                            virtualController = nil
                        }
                    #endif
                }
                .toolbar {
                    ToolbarItems(
                        activeGame: self.$activeGame, emulationContext: self.$emulationContext)
                }
                .navigationBarBackButtonHidden()
        }
        #if os(macOS)
            .windowToolbarFullScreenVisibility(.onHover)
        #endif
        .navigationTitle(navigationTitle)
    }
}

import SwiftUI

struct ContentView: View {
    @Binding var activeGame: Game?
    @Binding var emulationContext: HydraEmulationContext?

    @State private var fps: Int = 0
    
    private var isRunning: Bool {
        activeGame != nil ? true : false
    }

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
                }
                .navigationBarBackButtonHidden()
        }
        .toolbar { 
            ToolbarItems(isRunning: isRunning)
        }
        .windowToolbarFullScreenVisibility(.onHover)
        .navigationTitle(navigationTitle)
    }
}

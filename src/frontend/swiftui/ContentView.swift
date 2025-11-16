import SwiftUI

struct ContentView: View {
    @Binding var activeGame: Game?
    @Binding var emulationContext: HydraEmulationContext?
    
    private var appVersion: String {
        Bundle.main.infoDictionary?["CFBundleShortVersionString"] as? String ?? "?"
    }
    
    private var gitVersion: String {
        Bundle.main.infoDictionary?["CFBundleVersion"] as? String ?? "?"
    }
    
    /// The navigation title (either game name or "Game List") plus version
    private var navigationTitle: String {
        let gameTitle = activeGame?.name ?? "Game List"
        return "Hydra v\(appVersion) (\(gitVersion)) | \(gameTitle)"
    }

    var body: some View {
        NavigationStack {
            GameListView(activeGame: self.$activeGame)
                .navigationDestination(item: self.$activeGame) { activeGame in
                    EmulationView(game: activeGame, emulationContext: self.$emulationContext)
                        .aspectRatio(CGSize(width: 16, height: 9), contentMode: .fit)
                }
        }
        .toolbar {
            ToolbarItems()
        }
        .windowToolbarFullScreenVisibility(.onHover)
        .navigationTitle(navigationTitle)
    }
}

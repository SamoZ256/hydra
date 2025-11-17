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
    
    private var navigationTitle: String {
        let gameTitle = activeGame?.name ?? ""
        var titleInsert = ""
        if activeGame?.name != nil { titleInsert = "  |  \(gameTitle)" }
        var gitInsert = ""
        if gitVersion != "" { gitInsert = " (\(gitVersion))"}
        return "Hydra v\(appVersion)\(gitInsert)\(titleInsert)"
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

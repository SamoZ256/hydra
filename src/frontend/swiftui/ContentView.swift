import SwiftUI

struct ContentView: View {
    @Binding var activeGame: Game?
    @Binding var emulationContext: HydraEmulationContext?

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
        .navigationTitle(activeGame?.name ?? "Hydra")
    }
}

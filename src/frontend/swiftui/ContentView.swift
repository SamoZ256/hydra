import SwiftUI

struct ContentView: View {
    var body: some View {
        NavigationStack {
            GameListView()
        }
        .toolbar {
            ToolbarItems()
        }
        .windowToolbarFullScreenVisibility(.onHover)
    }
}

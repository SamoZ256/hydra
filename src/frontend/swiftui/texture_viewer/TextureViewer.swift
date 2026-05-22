import SwiftUI

struct TextureViewer: View {
    @EnvironmentObject var globalState: GlobalState

    // TODO: TextureMemoryListView
    var body: some View {
        if globalState.system != nil {
            TextureListView()
        } else {
            Text("Emulation not in progress")
        }
    }
}

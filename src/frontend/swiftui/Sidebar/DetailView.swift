import SwiftUI

struct DetailView: View {
    @Binding var selection: SidebarSelectionState?

    var body: some View {
        if let state = selection {
            switch state {
            case .nowPlaying:
                NowPlayingView()
            case .gamelist:
                GameListView()
            }
        }
    }
}
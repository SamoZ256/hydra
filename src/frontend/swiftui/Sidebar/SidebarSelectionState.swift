import Foundation

enum SidebarSelectionState: Identifiable, CaseIterable, Hashable {

    case nowPlaying
    case gamelist

    var id: String {
        switch self {
        case .nowPlaying:
            "nowPlaying"
        case .gamelist:
            "gamelist"
        }
    }

    var displayName: String {
        switch self {
        case .nowPlaying:
            "Now Playing"
        case .gamelist:
            "Game List"
        }
    }

    var iconName: String {
        switch self {
        case .nowPlaying:
            "play"
        case .gamelist:
            "rectangle.grid.2x2"
        }
    }
}

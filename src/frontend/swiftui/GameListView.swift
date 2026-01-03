import SwiftUI

enum ViewMode: Int {
    case list
    case grid
}

struct GameListView: View {
    @EnvironmentObject var globalState: GlobalState

    @Binding var viewMode: Int

    @State private var games: [Game] = []

    private let gridColumns = [
        GridItem(.adaptive(minimum: 180), spacing: 16)
    ]

    var body: some View {
        VStack {
            switch ViewMode(rawValue: viewMode) {
            case .list:
                List {
                    ForEach(games.indices, id: \.self) { index in
                        ClickableListItem(onClick: {
                            globalState.activeGame = games[index]
                        }) {
                            ListGamePreview(game: games[index])
                        }
                        .padding(.vertical, 16)
                    }
                }
            case .grid:
                ScrollView {
                    LazyVGrid(columns: gridColumns, spacing: 4) {
                        ForEach(games.indices, id: \.self) { index in
                            ClickableListItem(onClick: {
                                globalState.activeGame = games[index]
                            }) {
                                GridGamePreview(game: games[index])
                            }
                            .frame(maxWidth: .infinity)
                            .frame(height: 180)
                            .padding()
                        }
                    }
                    .padding()
                    .frame(maxWidth: .infinity)
                }
            case .none:
                Text("ERROR")
            }
        }
        .onAppear {
            games = GameListView.createGameList(gamePaths: globalState.gamePaths)
        }
        .onChange(of: globalState.gamePaths) { _, newValue in
            games = GameListView.createGameList(gamePaths: newValue)
        }
    }

    static func createGameList(gamePaths: [String]) -> [Game] {
        // Get all games
        var games: [Game] = []
        for gamePath in gamePaths {
            do {
                let url = try resolveUrl(URL(fileURLWithPath: gamePath))

                try processUrl(games: &games, url: url)
            } catch {
                // TODO: error popup
                print("Failed to load game path \(gamePath): \(error)")
            }
        }

        // Sort games by name
        games.sort { $0.name.caseInsensitiveCompare($1.name) == .orderedAscending }

        return games
    }

    static func processUrl(games: inout [Game], url: URL) throws {
        // Check if the URL is a game
        if url.pathExtension != "" {
            tryAddGame(games: &games, url: url)
            return
        }

        // Check if the URL is a directory
        var isDirectory: ObjCBool = false
        guard
            FileManager.default.fileExists(
                atPath: url.path(percentEncoded: false), isDirectory: &isDirectory)
        else {
            // TODO: error popup
            print("Game path \"\(url)\" does not exist")
            return
        }

        if !isDirectory.boolValue {
            return
        }

        let urls = try FileManager.default.contentsOfDirectory(
            at: url, includingPropertiesForKeys: nil)
        for url in urls {
            try processUrl(games: &games, url: url)
        }
    }

    static func tryAddGame(games: inout [Game], url: URL) {
        do {
            let game = try Game(url: url)
            games.append(game)
        } catch {
            // TODO: error popup
            print("Failed to load game path \(url.path(percentEncoded: false)): \(error)")
            return
        }
    }
}

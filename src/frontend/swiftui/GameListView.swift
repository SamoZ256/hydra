import SwiftUI

enum ViewMode: Int {
    case list
    case grid
}

struct GameListView: View {
    @EnvironmentObject var globalState: GlobalState

    @Binding var viewMode: ViewMode

    @State private var games: [Game] = []

    private let gridColumns = [
        GridItem(.adaptive(minimum: 180), spacing: 16)
    ]

    var body: some View {
        VStack {
            switch viewMode {
            case .list:
                List {
                    ForEach(games.indices, id: \.self) { index in
                        GamePreviewContainer<ListGamePreview>(game: games[index])
                            .padding(.vertical, 16)
                    }
                }
            case .grid:
                ScrollView {
                    LazyVGrid(columns: gridColumns, spacing: 4) {
                        ForEach(games.indices, id: \.self) { index in
                            GamePreviewContainer<GridGamePreview>(game: games[index])
                                .frame(maxWidth: .infinity)
                                .frame(height: 180)
                                .padding()
                        }
                    }
                    .padding()
                    .frame(maxWidth: .infinity)
                }
            }
        }
        .onAppear {
            games = GameListView.createGameList(gamePaths: globalState.gamePaths, loaderPluginManager: self.globalState.loaderPluginManager)
        }
        .onChange(of: globalState.gamePaths) { _, newValue in
            games = GameListView.createGameList(gamePaths: newValue, loaderPluginManager: self.globalState.loaderPluginManager)
        }
        .onChange(of: globalState.loaderPluginRefreshID) { _, _ in
            games = GameListView.createGameList(gamePaths: globalState.gamePaths, loaderPluginManager: self.globalState.loaderPluginManager)
        }
    }

    static func createGameList(gamePaths: [String], loaderPluginManager: HydraLoaderPluginManager?) -> [Game] {
        // Get all games
        var games: [Game] = []
        for gamePath in gamePaths {
            do {
                let url = try resolveUrl(URL(fileURLWithPath: gamePath))

                try processUrl(games: &games, url: url, loaderPluginManager: loaderPluginManager)
            } catch {
                // TODO: error popup
                print("Failed to load game path \(gamePath): \(error)")
            }
        }

        // Sort games by name
        games.sort { $0.name.caseInsensitiveCompare($1.name) == .orderedAscending }

        return games
    }

    static func processUrl(games: inout [Game], url: URL, loaderPluginManager: HydraLoaderPluginManager?) throws {
        // Check if the URL is a game
        if url.pathExtension != "" {
            tryAddGame(games: &games, url: url, loaderPluginManager: loaderPluginManager)
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
            try processUrl(games: &games, url: url, loaderPluginManager: loaderPluginManager)
        }
    }

    static func tryAddGame(games: inout [Game], url: URL, loaderPluginManager: HydraLoaderPluginManager?) {
        do {
            let game = try Game(url: url, loaderPluginManager: loaderPluginManager)
            games.append(game)
        } catch {
            // TODO: error popup
            print("Failed to load game path \(url.path(percentEncoded: false)): \(error)")
            return
        }
    }
}

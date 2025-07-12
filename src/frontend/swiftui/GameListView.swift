import SwiftUI

struct GameListView: View {
    @Binding var activeGame: Game?

    @State private var games: [Game] = []

    var body: some View {
        List {
            ForEach(self.games.indices, id: \.self) { index in
                GamePreview(game: games[index], activeGame: self.$activeGame)
                    .padding(.vertical, 8)
            }
        }
        //.navigationTitle("")
        .onAppear {
            load()
        }
        .onDisappear {
            for game in games {
                hydra_loader_destroy(game.loader)
            }
        }
    }

    func load() {
        // Game paths
        let gamePathsOption = hydra_config_get_game_paths()
        for i in 0..<hydra_string_array_option_get_count(gamePathsOption) {
            if let gamePathRaw = hydra_string_array_option_get(gamePathsOption, UInt32(i)) {
                let gamePath = String(cString: gamePathRaw)

                // TODO: do all of this with URLs
                let fileManager = FileManager.default

                var isDirectory: ObjCBool = false
                guard fileManager.fileExists(atPath: gamePath, isDirectory: &isDirectory)
                else {
                    print("Game path \"\(gamePath)\" does not exist")
                    continue
                }

                if !isDirectory.boolValue {
                    tryAddGame(path: gamePath)
                } else {
                    // Iterate recursively over the game directories
                    guard
                        let enumerator = fileManager.enumerator(
                            at: URL(fileURLWithPath: gamePath), includingPropertiesForKeys: nil)
                    else {
                        // TODO: error popup
                        print("Invalid game directory \(gamePath)")
                        return
                    }

                    while let url = enumerator.nextObject() as? URL {
                        tryAddGame(path: url.path)
                    }
                }
            }
        }
    }

    func tryAddGame(path: String) {
        // TODO: handle this differently
        if !path.hasSuffix("nro") && !path.hasSuffix("nso")
            && !path.hasSuffix("nca")
        {
            return
        }

        guard let game = createGameFromFile(path: path) else {
            return
        }

        self.games.append(game)
    }
}

import SwiftUI

struct GameListView: View {
    @State private var games: [Game] = []

    var body: some View {
        List {
            ForEach(self.games.indices, id: \.self) { index in
                GamePreview(game: games[index])
                    .padding(.vertical, 8)
            }
        }
        //.navigationTitle("")
        .onAppear {
            load()
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
                    let fileManager = FileManager.default
                    guard let enumerator = fileManager.enumerator(atPath: gamePath) else {
                        // TODO: error popup
                        print("Invalid game directory \(gamePath)")
                        return
                    }

                    while let filename = enumerator.nextObject() as? String {
                        // HACK: get the full path (only works for the root directory)
                        let path = "\(gamePath)/\(filename)"
                        tryAddGame(path: path)
                    }
                }
            }
        }
    }

    func tryAddGame(path: String) {
        // TODO: ask the core for supported extensions
        if !path.hasSuffix("nro") && !path.hasSuffix("nso")
            && !path.hasSuffix("nca")
        {
            return
        }

        self.games.append(Game(path: path))
    }
}

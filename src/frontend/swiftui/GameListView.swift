import SwiftUI

struct GameListView: View {
    @Binding var activeGame: Game?

    @State private var games: [Game] = []

    var body: some View {
        List {
            ForEach(self.games.indices, id: \.self) { index in
                GamePreview(game: self.games[index], activeGame: self.$activeGame)
                    .padding(.vertical, 8)
            }
        }
        //.navigationTitle("")
        .onAppear {
            load()
        }
        .onDisappear {
            games.removeAll()
        }
    }

    func load() {
        // Game paths
        let gamePathsOption = hydraConfigGetGamePaths()
        for i in 0..<gamePathsOption.count {
            let gamePath = gamePathsOption.get(at: i)
            do {
                let url = try resolveUrl(URL(fileURLWithPath: gamePath))

                var isDirectory: ObjCBool = false
                guard FileManager.default.fileExists(atPath: url.path, isDirectory: &isDirectory)
                else {
                    // TODO: error popup
                    print("Game path \"\(url)\" does not exist")
                    continue
                }

                if !isDirectory.boolValue {
                    tryAddGame(url: url)
                } else {
                    // Iterate recursively over the game directories
                    guard
                        let enumerator = FileManager.default.enumerator(
                            at: url, includingPropertiesForKeys: nil)
                    else {
                        // TODO: error popup
                        print("Invalid game directory \(gamePath)")
                        return
                    }

                    while let url = enumerator.nextObject() as? URL {
                        tryAddGame(url: url)
                    }
                }
            } catch {
                // TODO: error popup
                print("Failed to load game path \(gamePath)")
            }
        }
    }

    func tryAddGame(url: URL) {
        // TODO: handle this better
        if url.pathExtension != "nro" && url.pathExtension != "nso" && url.pathExtension != "nca" {
            return
        }

        guard let game = createGameFromFile(url: url) else {
            return
        }

        self.games.append(game)
    }
}

func createGameFromFile(path: String) -> Game? {
    guard let loader = hydra_create_loader_from_file(path) else {
        return nil
    }

    // TODO: get from NACP
    let name = URL(fileURLWithPath: path).deletingPathExtension().lastPathComponent

    return Game(loader: loader, name: name)
}

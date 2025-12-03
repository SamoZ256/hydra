import Foundation

func createGameFromFile(url: URL) -> Game? {
    let loader = HydraLoader(path: url.path)

    // Get name and author
    var name = ""
    var author = ""
    let nacp = loader.loadNacp()
    if let nacp = nacp {
        let title = nacp.title
        name = title.name
        author = title.author
    } else {
        name = url.deletingPathExtension().lastPathComponent
        author = "Unknown"
    }

    return Game(url: url, loader: loader, name: name, author: author)
}

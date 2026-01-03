import Foundation

struct Game: Hashable {
    var url: URL
    var loader: HydraLoader
    var name: String
    var author: String
    var version: String

    init(url: URL, loader: HydraLoader, name: String, author: String, version: String) {
        self.url = url
        self.loader = loader
        self.name = name
        self.author = author
        self.version = version
    }

    init(url: URL) throws {
        let loader = try HydraLoader(path: url.path(percentEncoded: false))

        // Get name and author
        var name = ""
        var author = ""
        var version = ""
        let nacp = loader.loadNacp()
        if let nacp = nacp {
            let title = nacp.title
            name = title.name
            author = title.author
            version = nacp.displayVersion
        } else {
            name = url.deletingPathExtension().lastPathComponent
            author = "Unknown"
            version = "0"
        }

        self.init(url: url, loader: loader, name: name, author: author, version: version)
    }
}

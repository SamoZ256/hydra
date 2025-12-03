import Foundation

struct Game: Hashable {
    var url: URL
    var loader: HydraLoader
    var name: String
    var author: String
}

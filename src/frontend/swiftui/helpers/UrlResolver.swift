import Foundation

func registerUrl(_ url: URL) throws {
    #if os(iOS)
        guard url.startAccessingSecurityScopedResource() else {
            // TODO
            throw NSError(
                domain: "", code: 123,
                userInfo: [
                    NSLocalizedDescriptionKey:
                        "Failed to access security scoped resource for URL \(url)"
                ])
        }

        defer { url.stopAccessingSecurityScopedResource() }

        let bookmarkData = try url.bookmarkData(
            options: .minimalBookmark, includingResourceValuesForKeys: nil, relativeTo: nil)

        UserDefaults.standard.set(bookmarkData, forKey: url.absoluteString)
    #endif
}

func resolveUrl(_ url: URL) throws -> URL {
    #if os(macOS)
        return url
    #else
        guard let bookmarkData = UserDefaults.standard.data(forKey: url.absoluteString)
        else {
            // TODO
            throw NSError(
                domain: "", code: 123,
                userInfo: [NSLocalizedDescriptionKey: "Bookmark not found for URL \(url)"])
        }

        var isStale = false
        guard
            let resolvedUrl = try URL(
                resolvingBookmarkData: bookmarkData, bookmarkDataIsStale: &isStale)
        else {
            // TODO
            throw NSError(
                domain: "", code: 123,
                userInfo: [NSLocalizedDescriptionKey: "Failed to resolve URL \(url)"])
        }

        guard !isStale else {
            // TODO
            throw NSError(
                domain: "", code: 123,
                userInfo: [NSLocalizedDescriptionKey: "Stale URL \(resolvedUrl)"])
        }

        return resolvedUrl
    #endif
}

import SwiftUI

struct GamePreview: View {
    let game: Game
    @Binding var activeGame: Game?

    var body: some View {
        ClickableListItem(onClick: {
            self.activeGame = self.game
        }) {
            HStack {
                if let nsImage = self.loadIconNS() {
                    Image(nsImage: nsImage)
                        .resizable()
                        .aspectRatio(contentMode: .fit)
                        .frame(maxWidth: 64, maxHeight: 64)  // TODO: don't hardcode
                } else {
                    VStack {
                        ZStack {
                            Image(systemName: "photo.fill")
                                .resizable()
                                .aspectRatio(contentMode: .fit)
                            Image(systemName: "line.diagonal")
                                .resizable()
                                .aspectRatio(contentMode: .fill)
                        }
                        Text("No Image")
                            .font(.system(size: 6))
                    }
                    .padding()
                    .background(.quaternary)
                    .frame(maxWidth: 64, maxHeight: 64)  // TODO: don't hardcode
                }
                Text(game.name)
                    .padding()
                // TODO: author?
            }
            .padding(4)
        }
    }

    private func loadIcon(width: inout UInt64, height: inout UInt64) -> CGImage? {
        guard let data = self.game.loader.loadIcon(width: &width, height: &height) else {
            return nil
        }

        defer { free(data) }

        guard
            let context = CGContext(
                data: data,
                width: Int(width),
                height: Int(height),
                bitsPerComponent: 8,
                bytesPerRow: Int(width) * 4,
                space: CGColorSpaceCreateDeviceRGB(),
                bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue
            )
        else {
            return nil
        }

        return context.makeImage()
    }

    private func loadIconNS() -> NSImage? {
        var width: UInt64 = 0
        var height: UInt64 = 0
        guard let cgImage = self.loadIcon(width: &width, height: &height) else {
            return nil
        }

        return NSImage(cgImage: cgImage, size: NSSize(width: Int(width), height: Int(height)))
    }
}

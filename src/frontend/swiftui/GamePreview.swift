import SwiftUI

struct GamePreview: View {
    let game: Game
    @Binding var activeGame: Game?

    @State private var lastClickTime: Date? = nil
    @State private var navigate = false

    var body: some View {
        HStack {
            if let nsImage = self.loadIconNS() {
                Image(nsImage: nsImage)
                    .resizable()
                    .aspectRatio(contentMode: .fit)
                    .frame(maxWidth: 64, maxHeight: 64)  // TODO: don't hardcode
            }
            Text(game.name)
                .padding()
        }
        .contentShape(Rectangle())
        .onTapGesture {
            if let lastTime = lastClickTime, Date().timeIntervalSince(lastTime) < 0.3 {
                activeGame = self.game
                lastClickTime = nil
            } else {
                lastClickTime = Date()
            }
        }
    }

    private func loadIcon(width: inout UInt64, height: inout UInt64) -> CGImage? {
        var data: UnsafeMutableRawPointer?
        hydra_loader_load_icon(self.game.loader, &data, &width, &height)
        guard let data = data else {
            return nil
        }

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

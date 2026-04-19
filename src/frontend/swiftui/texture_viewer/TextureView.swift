import SwiftUI

struct TextureView: View {
    let storage: HydraTextureStorage

    private let type: HydraTextureType
    private let format: HydraTextureFormat
    private let width: UInt32
    private let height: UInt32
    private let depth: UInt32

    init(storage: HydraTextureStorage) {
        self.storage = storage

        let descriptor = storage.descriptor
        self.type = descriptor.type
        self.format = descriptor.format
        self.width = descriptor.width
        self.height = descriptor.height
        self.depth = descriptor.depth
    }

    var body: some View {
        HStack {
            // TODO: preview

            VStack {
                Text("\(self.width)x\(self.height)\(self.depth == 1 ? "" : "x\(self.depth)")")
                    .bold()
                Text("\(self.type.rawValue)")  // TODO: .description
                Text("\(self.format.rawValue)")  // TODO: .description
            }
        }
    }
}

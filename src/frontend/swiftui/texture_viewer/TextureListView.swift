import SwiftUI

enum SizeFormat: Int {
    case decimal, hex, byteCount

    func format(_ nb: UInt64) -> String {
        switch (self) {
            case .decimal:
                return nb.formatted()
            case .hex:
                return "0x" + String(nb, radix: 16, uppercase: false)
            case .byteCount:
                return ByteCountFormatter.string(fromByteCount: Int64(nb), countStyle: .binary)
        }
    }
}

struct TextureSizeView: View {
    let size: UInt64
    @Binding var sizeFormat: SizeFormat

    var body: some View {
        Text(sizeFormat.format(size))
            .font(.system(.body, design: .monospaced)) // Better for hex
            .frame(maxWidth: .infinity, alignment: .trailing)
            .contentShape(Rectangle()) // Clickable
            .contextMenu {
                Picker("Size Format", selection: self.$sizeFormat) {
                    Text("Decimal").tag(SizeFormat.decimal)
                    Text("Hex").tag(SizeFormat.hex)
                    Text("Byte Count").tag(SizeFormat.byteCount)
                }

                Divider()

                Button("Copy Value") {
                    NSPasteboard.general.clearContents()
                    NSPasteboard.general.setString(sizeFormat.format(size), forType: .string)
                }
            }
    }
}

struct TextureListView: View {
    @EnvironmentObject var globalState: GlobalState

    @State private var refreshID = 0

    @AppStorage("sizeFormat") private var sizeFormat: SizeFormat = .decimal

    @State private var textures: [HydraTextureStorage] = []

    @State private var sortOrder = [KeyPathComparator(\HydraTextureStorage.descriptor.format)]
    private var sortedTextures: [HydraTextureStorage] {
        return textures.sorted(using: sortOrder)
    }

    var body: some View {
        ZStack {
            Table(self.sortedTextures, sortOrder: self.$sortOrder) {
                TableColumn("Dimensions", value: \.descriptor.width) { texture in // TODO: use whole size as the value
                    Text("\(texture.descriptor.width) x \(texture.descriptor.height)\(texture.descriptor.depth == 1 ? "" : " x \(texture.descriptor.depth)")")
                }
                TableColumn("Type", value: \.descriptor.type) { texture in
                    Text(texture.descriptor.type.description)
                }
                TableColumn("Format", value: \.descriptor.format) { texture in
                    Text(texture.descriptor.format.description)
                }
                TableColumn("Levels", value: \.descriptor.levelCount) { texture in
                    Text(String(texture.descriptor.levelCount))
                }
                TableColumn("Layers", value: \.descriptor.layerCount) { texture in
                    Text(String(texture.descriptor.layerCount))
                }
                TableColumn("Layer Size", value: \.descriptor.layerSize) { texture in
                    TextureSizeView(size: texture.descriptor.layerSize, sizeFormat: self.$sizeFormat)
                }
                TableColumn("Total Size", value: \.descriptor.size) { texture in
                    TextureSizeView(size: texture.descriptor.size, sizeFormat: self.$sizeFormat)
                }
            }
                .id("\(refreshID)")  // Unique ID per refresh

            // TODO: add an option to refresh at regular intervals or any time a change happens?
            HStack {
                VStack {
                    Spacer()

                    Button(action: {
                        load()
                        refreshID += 1
                    }) {
                        Image(systemName: "arrow.clockwise")
                            .font(.title2)
                            .foregroundColor(.blue)
                            .padding()
                    }
                    .padding()
                }

                Spacer()
            }
        }
        .onAppear {
            load()
        }
    }

    func load() {
        globalState.system!.textureCacheLock()

        self.textures.removeAll()
        for i in 0..<globalState.system!.textureCacheGetTextureMemoryCount() {
            let mem = globalState.system!.textureCacheGetTextureMemory(at: i)
            for j in 0..<mem.textureGroupCount {
                let group = mem.getTextureGroup(at: j)
                for k in 0..<group.textureStorageCount {
                    let storage = group.getTextureStorage(at: k)
                    self.textures.append(storage)
                }
            }
        }

        globalState.system!.textureCacheUnlock()
    }
}

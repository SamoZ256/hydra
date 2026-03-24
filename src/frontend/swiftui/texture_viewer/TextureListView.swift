import SwiftUI

struct TextureListView: View {
    @State private var refreshID = 0

    @State private var textures: [HydraTextureGroup] = []

    var body: some View {
        ZStack {
            List {
                ForEach(Array(self.textures.enumerated()), id: \.offset) { index, group in
                    TextureView(group: group)
                        .id("\(refreshID)-\(index)")  // Unique ID per refresh
                }
            }

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
        hydraTextureCacheLock()

        self.textures.removeAll()
        for i in 0..<hydraTextureCacheGetTextureMemoryCount() {
            let mem = hydraTextureCacheGetTextureMemory(at: i)
            for j in 0..<mem.sparseTextureCount {
                let sparse_tex = mem.getSparseTexture(at: j)
                for k in 0..<sparse_tex.textureGroupCount {
                    let group = sparse_tex.getTextureGroup(at: k)
                    self.textures.append(group)
                }
            }
        }

        hydraTextureCacheUnlock()
    }
}

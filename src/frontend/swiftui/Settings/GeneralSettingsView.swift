import SwiftUI
import UniformTypeIdentifiers

struct GeneralSettingsView: View {
    @State private var gamePaths: [String] = []
    @State private var patchPaths: [String] = []

    private let switchType = UTType(exportedAs: "com.samoz256.switch-document", conformingTo: .data)
    private let hatchType = UTType(exportedAs: "com.samoz256.hatch-document", conformingTo: .data)

    var body: some View {
        Spacer()
        HStack {
            Spacer()
            Form {
                Section("Game Paths") {
                    // Text("Game paths")
                    EditablePathList(
                        allowedContentTypes: [.folder, self.switchType],
                        items: self.$gamePaths
                    )
                    .onChange(of: self.gamePaths) { _, newValue in
                        let gamePathsOption = hydraConfigGetGamePaths()
                        gamePathsOption.resize(newCount: newValue.count)
                        for i in 0..<newValue.count {
                            gamePathsOption.set(at: i, value: newValue[i])
                        }
                    }
                }
                
                Section("Patch paths") {
                    // Text("Patch paths")
                    EditablePathList(
                        allowedContentTypes: [.folder, self.hatchType], items: self.$patchPaths
                    )
                    .onChange(of: self.patchPaths) { _, newValue in
                        let patchPathsOption = hydraConfigGetPatchPaths()
                        patchPathsOption.resize(newCount: newValue.count)
                        for i in 0..<newValue.count {
                            patchPathsOption.set(at: i, value: newValue[i])
                        }
                    }
                }
            }
            .onAppear {
                let gamePathsOption = hydraConfigGetGamePaths()
                self.gamePaths = []
                for i in 0..<gamePathsOption.count {
                    self.gamePaths.append(gamePathsOption.get(at: i))
                }
    
                let patchPathsOption = hydraConfigGetPatchPaths()
                self.patchPaths = []
                for i in 0..<patchPathsOption.count {
                    self.patchPaths.append(patchPathsOption.get(at: i))
                }
            }
            .formStyle(.grouped)
            Spacer()
        }
        Spacer()
    }
}

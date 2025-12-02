import SwiftUI
import UniformTypeIdentifiers

struct ToolbarItems: ToolbarContent {
    @State private var isFilePickerPresented: Bool = false
    @Binding var activeGame: Game?
    @Binding var emulationContext: HydraEmulationContext?
    
    private var isRunning: Bool {
        activeGame != nil ? true : false
    }

    private let switchType = UTType(exportedAs: "com.samoz256.switch-document", conformingTo: .data)
    

    var body: some ToolbarContent {
        ToolbarItemGroup(placement: .principal) {
            Button("Play", systemImage: "play") {}.disabled(isRunning)
            Button("Stop", systemImage: "stop") {
                guard let emulationContext = self.emulationContext else { return }
                emulationContext.requestStop()
                // TODO: wait a bit?
                emulationContext.forceStop()
                self.activeGame = nil
            }
            .disabled(!isRunning)
            Button("Pause", systemImage: "pause") {}.disabled(!isRunning)
        }
        
        ToolbarItemGroup(placement: .automatic) {
            Button("Add Game Path", systemImage: "plus") {
                self.isFilePickerPresented = true
            }
            .fileImporter(
                isPresented: self.$isFilePickerPresented,
                allowedContentTypes: [.folder, self.switchType],
                allowsMultipleSelection: true
            ) { result in
                switch result {
                case .success(let fileURLs):
                    for fileURL in fileURLs {
                        let gamePathsOption = hydraConfigGetGamePaths()
                        gamePathsOption.append(
                            value: fileURL.path)

                        hydraConfigSerialize()
                    }
                case .failure(let error):
                    print(error)
                }
            }
        }
    }

}

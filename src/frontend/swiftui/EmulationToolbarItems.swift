import SwiftUI

struct EmulationToolbarItems: ToolbarContent {
    @EnvironmentObject var globalState: GlobalState

    @State private var isRunning: Bool = false

    var body: some ToolbarContent {
        #if os(macOS)
            ToolbarItemGroup(placement: .principal) {
                if isRunning {
                    Button("Pause", systemImage: "pause") {
                        guard let emulationContext = globalState.emulationContext else { return }
                        emulationContext.pause()
                        isRunning = false
                    }
                } else {
                    Button("Resume", systemImage: "play") {
                        guard let emulationContext = globalState.emulationContext else { return }
                        emulationContext.resume()
                        isRunning = true
                    }
                }
                Button("Stop", systemImage: "stop") {
                    globalState.isStopping = true
                }
                .onAppear {
                    isRunning = globalState.emulationContext!.isRunning()
                }
            }
            
            ToolbarItemGroup(placement: .confirmationAction) {
                HStack {
                    Button("Console Mode", systemImage: "inset.filled.tv") {
                        globalState.isHandheldMode = !globalState.isHandheldMode
                        hydraConfigGetHandheldMode().pointee = globalState.isHandheldMode
                        hydraConfigSerialize()
                        
                        guard let emulationContext = globalState.emulationContext else { return }
                        emulationContext.notifyOperationModeChanged()
                    }
                    .disabled(!globalState.isHandheldMode)
                    
                    Divider()
                    
                    Button("Handheld Mode", systemImage: "formfitting.gamecontroller.fill") {
                        globalState.isHandheldMode = !globalState.isHandheldMode
                        hydraConfigGetHandheldMode().pointee = globalState.isHandheldMode
                        hydraConfigSerialize()
                        
                        guard let emulationContext = globalState.emulationContext else { return }
                        emulationContext.notifyOperationModeChanged()
                    }
                    .disabled(globalState.isHandheldMode)
                }
            }
        #else
            // TODO: options
            ToolbarItemGroup(placement: .principal) {}
        #endif
    }

}

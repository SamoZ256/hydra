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
            ToolbarItemGroup(placement: .principal) {
                
            }
        #else
            // TODO: options
            ToolbarItemGroup(placement: .principal) {}
        #endif
    }

}

import SwiftUI

struct EmulationToolbarItems: ToolbarContent {
    @Binding var emulationState: EmulationState

    @State private var isRunning: Bool = false

    var body: some ToolbarContent {
        #if os(macOS)
            ToolbarItemGroup(placement: .principal) {
                if isRunning {
                    Button("Pause", systemImage: "pause.fill") {
                        guard let emulationContext = emulationState.emulationContext else { return }
                        emulationContext.pause()
                        isRunning = false
                    }
                } else {
                    Button("Resume", systemImage: "play.fill") {
                        guard let emulationContext = emulationState.emulationContext else { return }
                        emulationContext.resume()
                        isRunning = true
                    }
                }
                
                Button("Stop", systemImage: "stop.fill") {
                    guard let emulationContext = emulationState.emulationContext else { return }
                    emulationContext.requestStop()
                    // TODO: wait a bit?
                    emulationContext.forceStop()
                    emulationState.activeGame = nil
                }
                .onAppear {
                    isRunning = emulationState.emulationContext!.isRunning()
                }
            }
        #endif
    }

}

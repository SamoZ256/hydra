import SwiftUI

struct EmulationView: View {
    let game: Game

    @State private var emulationContext: UnsafeMutableRawPointer? = nil

    var body: some View {
        MetalView(emulationContext: self.$emulationContext)
            .onAppear {
                startEmulation()
            }
            .onDisappear {
                stopEmulation()
            }
    }

    func startEmulation() {
        guard let emulationContext = hydra_create_emulation_context() else {
            // TODO: error popup
            print("Failed to create emulation context")
            return
        }

        self.emulationContext = emulationContext
        hydra_emulation_context_load(emulationContext, game.loader)
        hydra_emulation_context_run(emulationContext)
    }

    func stopEmulation() {
        if let emulationContext = self.emulationContext {
            hydra_emulation_context_destroy(emulationContext)
        }
    }
}

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
        guard let emulationContext = hydra_emulation_context_create() else {
            // TODO: error popup
            print("Failed to create emulation context")
            return
        }

        self.emulationContext = emulationContext
        hydra_emulation_context_load_rom(emulationContext, game.path)
        hydra_emulation_context_run(emulationContext)
    }

    func stopEmulation() {
        if let emulationContext = self.emulationContext {
            hydra_emulation_context_destroy(emulationContext)
        }
    }
}

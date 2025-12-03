import SwiftUI

struct EmulationView: View {
    let game: Game

    @Binding var emulationContext: HydraEmulationContext?
    @Binding var fps: Int

    var body: some View {
        MetalView(emulationContext: self.$emulationContext, fps: $fps)
            .onAppear {
                self.emulationContext = HydraEmulationContext()
                self.emulationContext!.loadAndStart(loader: game.loader)
            }
            .onDisappear {
                // TODO: stop?
                self.emulationContext = nil
                fps = 0
            }
    }
}

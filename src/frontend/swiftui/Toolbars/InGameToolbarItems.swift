import SwiftUI

struct InGameToolbarItems: ToolbarContent {
	@Binding var activeGame: Game?
	@Binding var emulationContext: HydraEmulationContext?

	private var isRunning: Bool {
		activeGame != nil ? true : false
	}

	var body: some ToolbarContent {
		#if os(macOS)
			ToolbarItemGroup(placement: .principal) {
				if isRunning {
					Button("Pause", systemImage: "pause.fill") {}
				} else {
					Button("Play", systemImage: "play.fill") {}
				}
				Button("Stop", systemImage: "stop.fill") {
					guard let emulationContext = self.emulationContext else { return }
					emulationContext.requestStop()
					// TODO: wait a bit?
					emulationContext.forceStop()
					self.activeGame = nil
				}
				.disabled(!isRunning)
				
			}
		#endif
	}

}

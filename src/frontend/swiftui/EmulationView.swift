import SwiftUI

struct EmulationView: View {
    @EnvironmentObject var globalState: GlobalState

    @Binding var fps: Int

    var body: some View {
        ZStack {
            MetalView(fps: $fps)
                .onAppear {
                    globalState.system = HydraSystem()
                    globalState.system!.loadAndStart(
                        loader: globalState.activeGame!.loader)
                }
                .onDisappear {
                    fps = 0
                }

            #if os(macOS)
                KeyCatcherView()
            #endif

            if globalState.isStopping {
                GameStopView()
            }
        }
    }
}

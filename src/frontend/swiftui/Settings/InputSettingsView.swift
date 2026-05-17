import SwiftUI

#if HYDRA_SDL_ENABLED
    let sdlEnabled = true
#else
    let sdlEnabled = false
#endif

struct InputSettingsView: View {
    @State private var inputBackend: HydraInputBackend = HYDRA_INPUT_BACKEND_INVALID

    var body: some View {
        Spacer()
        HStack {
            Spacer()
            Form {
                Section {
                    Picker("Input backend", selection: self.$inputBackend.rawValue) {
                        Text("Apple GameController (recommended)")
                            .tag(HYDRA_INPUT_BACKEND_APPLE_GAME_CONTROLLER.rawValue)
                        Text("SDL")
                            .tag(HYDRA_INPUT_BACKEND_SDL.rawValue)
                            .selectionDisabled(!sdlEnabled)
                    }
                    .onChange(of: self.inputBackend.rawValue) { _, newValue in
                        hydraConfigGetInputBackend().pointee = newValue
                    }
                }

                Section {
                    // TODO: input profiles
                    Text("Input profile configuration is not yet supproted")
                }
            }
            .formStyle(.grouped)
            .onAppear {
                self.inputBackend.rawValue = hydraConfigGetInputBackend().pointee
            }
            Spacer()
        }
        Spacer()
    }
}

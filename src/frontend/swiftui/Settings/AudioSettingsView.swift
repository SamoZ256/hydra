import SwiftUI

struct AudioSettingsView: View {
    @State private var audioBackend: HydraAudioBackend = HYDRA_AUDIO_BACKEND_INVALID

    var body: some View {
        Spacer()
        HStack {
            Spacer()
            Form {
                Section {
                    Picker("Audio backend", selection: self.$audioBackend.rawValue) {
                        Text("Null")
                            .tag(HYDRA_AUDIO_BACKEND_NULL.rawValue)
                        Text("Cubeb (recommended)")
                            .tag(HYDRA_AUDIO_BACKEND_CUBEB.rawValue)
                    }
                    .onChange(of: self.audioBackend.rawValue) { _, newValue in
                        var audioBackendOption = hydraConfigGetAudioBackend()
                        audioBackendOption.value = newValue
                    }
                }
            }
            .formStyle(.grouped)
            .padding(24)
            .padding(.horizontal, 64)
            .onAppear {
                let audioBackendOption = hydraConfigGetAudioBackend()
                self.audioBackend.rawValue = audioBackendOption.value
            }
            Spacer()
        }
        Spacer()
    }
}

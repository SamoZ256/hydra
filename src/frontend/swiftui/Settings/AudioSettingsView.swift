import SwiftUI

#if HYDRA_CUBEB_ENABLED
let cubebEnabled = true
#else
let cubebEnabled = false
#endif

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
                        Text(cubebEnabled ? "Cubeb (recommended)" : "Cubeb")
                            .tag(HYDRA_AUDIO_BACKEND_CUBEB.rawValue)
                            .selectionDisabled(!cubebEnabled)
                    }
                    .onChange(of: self.audioBackend.rawValue) { _, newValue in
                        hydraConfigGetAudioBackend().pointee = newValue
                    }
                }
            }
            .formStyle(.grouped)
            .onAppear {
                self.audioBackend.rawValue = hydraConfigGetAudioBackend().pointee
            }
            Spacer()
        }
        Spacer()
    }
}

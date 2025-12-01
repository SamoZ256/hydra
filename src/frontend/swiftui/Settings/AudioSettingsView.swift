import SwiftUI

struct AudioSettingsView: View {
    @State var audioBackend: HydraAudioBackend = HYDRA_AUDIO_BACKEND_INVALID

    var body: some View {
        GeometryReader { geo in
            GroupBox {
                VStack {
                    Picker("Audio backend", selection: self.$audioBackend.rawValue) {
                        Text("Null (recommended)").tag(
                            HYDRA_AUDIO_BACKEND_NULL.rawValue)
                        Text("Cubeb (experimental)").tag(HYDRA_AUDIO_BACKEND_CUBEB.rawValue)
                    }
                    .onChange(of: self.audioBackend.rawValue) { _, newValue in
                        var audioBackendOption = hydraConfigGetAudioBackend()
                        audioBackendOption.value = newValue
                    }
                }
                .frame(width: geo.size.width, height: geo.size.height)
            }
        }
        .onAppear {
            let audioBackendOption = hydraConfigGetAudioBackend()
            self.audioBackend.rawValue = audioBackendOption.value
        }
    }
}

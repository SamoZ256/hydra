import SwiftUI

struct CpuSettingsView: View {
    @State var cpuBackend: HydraCpuBackend = HYDRA_CPU_BACKEND_INVALID

    var body: some View {
        VStack {
            Picker("CPU backend", selection: self.$cpuBackend.rawValue) {
                Text("Apple Hypervisor (recommended)").tag(
                    HYDRA_CPU_BACKEND_APPLE_HYPERVISOR.rawValue)
                Text("dynarmic").tag(HYDRA_CPU_BACKEND_DYNARMIC.rawValue)
            }
            .onChange(of: self.cpuBackend.rawValue) { _, newValue in
                var cpuBackendOption = hydraConfigGetCpuBackend()
                cpuBackendOption.value = newValue
            }
        }
        .onAppear {
            let cpuBackendOption = hydraConfigGetCpuBackend()
            self.cpuBackend.rawValue = cpuBackendOption.value
        }
    }
}

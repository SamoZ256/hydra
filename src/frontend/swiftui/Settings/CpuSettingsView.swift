import SwiftUI

struct CpuSettingsView: View {
    @State private var cpuBackend: HydraCpuBackend = HYDRA_CPU_BACKEND_INVALID

    var body: some View {
        Spacer()
        HStack {
            Spacer()
            Form {
                Section {
                    Picker("CPU backend", selection: self.$cpuBackend.rawValue) {
                        Text("Apple Hypervisor (recommended)")
                            .tag(HYDRA_CPU_BACKEND_APPLE_HYPERVISOR.rawValue)
                        Text("dynarmic")
                            .tag(HYDRA_CPU_BACKEND_DYNARMIC.rawValue)
                    }
                    .onChange(of: self.cpuBackend.rawValue) { _, newValue in
                        var cpuBackendOption = hydraConfigGetCpuBackend()
                        cpuBackendOption.value = newValue
                    }
                }

            }
            .formStyle(.grouped)
            .onAppear {
                let cpuBackendOption = hydraConfigGetCpuBackend()
                self.cpuBackend.rawValue = cpuBackendOption.value
            }
            Spacer()
        }
        Spacer()
    }
}

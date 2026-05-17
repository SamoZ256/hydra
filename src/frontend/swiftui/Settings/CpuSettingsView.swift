import SwiftUI

#if HYDRA_HYPERVISOR_ENABLED
let hypervisorEnabled = true
#else
let hypervisorEnabled = false
#endif

struct CpuSettingsView: View {
    @State private var cpuBackend: HydraCpuBackend = HYDRA_CPU_BACKEND_INVALID

    var body: some View {
        Spacer()
        HStack {
            Spacer()
            Form {
                Section {
                    Picker("CPU backend", selection: self.$cpuBackend.rawValue) {
                        Text(hypervisorEnabled ? "Apple Hypervisor (recommended)" : "Apple Hypervisor")
                            .tag(HYDRA_CPU_BACKEND_APPLE_HYPERVISOR.rawValue)
                            .selectionDisabled(!hypervisorEnabled)
                        Text("dynarmic")
                            .tag(HYDRA_CPU_BACKEND_DYNARMIC.rawValue)
                    }
                    .onChange(of: self.cpuBackend.rawValue) { _, newValue in
                        hydraConfigGetCpuBackend().pointee = newValue
                    }
                }

            }
            .formStyle(.grouped)
            .onAppear {
                self.cpuBackend.rawValue = hydraConfigGetCpuBackend().pointee
            }
            Spacer()
        }
        Spacer()
    }
}

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
        }
        .onAppear {
            load()
        }
        .onDisappear {
            save()
        }
    }

    func load() {
        let cpuBackendOption = hydra_config_get_cpu_backend()
        self.cpuBackend.rawValue = hydra_u32_option_get(cpuBackendOption)
    }

    func save() {
        let cpuBackendOption = hydra_config_get_cpu_backend()
        hydra_u32_option_set(cpuBackendOption, self.cpuBackend.rawValue)
    }
}

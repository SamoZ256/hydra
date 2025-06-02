import SwiftUI

struct GraphicsSettingsView: View {
    @State var gpuRenderer: HydraGpuRenderer = HYDRA_GPU_RENDERER_INVALID
    @State var shaderBackend: HydraShaderBackend = HYDRA_SHADER_BACKEND_INVALID

    var body: some View {
        VStack {
            Picker("GPU renderer", selection: self.$gpuRenderer.rawValue) {
                Text("Metal (recommended)").tag(HYDRA_GPU_RENDERER_METAL.rawValue)
            }
            Picker("Shader backend", selection: self.$shaderBackend.rawValue) {
                Text("MSL (recommended)").tag(HYDRA_SHADER_BACKEND_MSL.rawValue)
                Text("AIR (broken)").tag(HYDRA_SHADER_BACKEND_AIR.rawValue)
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
        let gpuRendererOption = hydra_config_get_gpu_renderer()
        self.gpuRenderer.rawValue = hydra_u32_option_get(gpuRendererOption)

        let shaderBackendOption = hydra_config_get_shader_backend()
        self.shaderBackend.rawValue = hydra_u32_option_get(shaderBackendOption)
    }

    func save() {
        let gpuRendererOption = hydra_config_get_gpu_renderer()
        hydra_u32_option_set(gpuRendererOption, self.gpuRenderer.rawValue)

        let shaderBackendOption = hydra_config_get_shader_backend()
        hydra_u32_option_set(shaderBackendOption, self.shaderBackend.rawValue)
    }
}

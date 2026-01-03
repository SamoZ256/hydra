import SwiftUI

struct GraphicsSettingsView: View {
    @State private var gpuRenderer: HydraGpuRenderer = HYDRA_GPU_RENDERER_INVALID
    @State private var shaderBackend: HydraShaderBackend = HYDRA_SHADER_BACKEND_INVALID
    @State private var displayResolution: HydraResolution = HYDRA_RESOLUTION_INVALID
    @State private var customDisplayResolution: hydra_uint2 = hydra_uint2(x: 0, y: 0)

    var body: some View {
        Spacer()
        HStack {
            Spacer()
            Form {
                Section {
                    Picker("Gpu renderer", selection: self.$gpuRenderer.rawValue) {
                        Text("Metal (recommended)").tag(HYDRA_GPU_RENDERER_METAL.rawValue)
                    }
                    .onChange(of: self.gpuRenderer.rawValue) { _, newValue in
                        hydraConfigGetGpuRenderer().pointee = newValue
                    }

                    Picker("Shader backend", selection: self.$shaderBackend.rawValue) {
                        Text("MSL (recommended)").tag(HYDRA_SHADER_BACKEND_MSL.rawValue)
                        Text("AIR (broken)").tag(HYDRA_SHADER_BACKEND_AIR.rawValue)
                    }
                    .onChange(of: self.shaderBackend.rawValue) { _, newValue in
                        hydraConfigGetShaderBackend().pointee = newValue
                    }

                    Picker("Display resolution", selection: self.$displayResolution.rawValue) {
                        Text("Auto (recommended)").tag(HYDRA_RESOLUTION_AUTO.rawValue)
                        Text("720p").tag(HYDRA_RESOLUTION_720P.rawValue)
                        Text("1080p").tag(HYDRA_RESOLUTION_1080P.rawValue)
                        Text("1440p").tag(HYDRA_RESOLUTION_1440P.rawValue)
                        Text("2160p").tag(HYDRA_RESOLUTION_2160P.rawValue)
                        Text("4320p").tag(HYDRA_RESOLUTION_4320P.rawValue)
                        Text("Auto exact (not recommended)").tag(
                            HYDRA_RESOLUTION_AUTO_EXACT.rawValue)
                        Text("Custom (not recommended)").tag(HYDRA_RESOLUTION_CUSTOM.rawValue)
                    }
                    .onChange(of: self.displayResolution.rawValue) { _, newValue in
                        hydraConfigGetDisplayResolution().pointee = newValue
                    }
                }

                if self.displayResolution == HYDRA_RESOLUTION_CUSTOM {
                    Section("Custom Resolution") {
                        TextField(
                            "Width", value: self.$customDisplayResolution.x,
                            format: .number)
                        TextField(
                            "Height", value: self.$customDisplayResolution.y,
                            format: .number
                        )
                        .onChange(of: self.customDisplayResolution) { _, newValue in
                            hydraConfigGetCustomDisplayResolution().pointee = newValue
                        }
                    }
                }
            }
            .formStyle(.grouped)
            .onAppear {
                self.gpuRenderer.rawValue = hydraConfigGetGpuRenderer().pointee
                self.shaderBackend.rawValue = hydraConfigGetShaderBackend().pointee
                self.displayResolution.rawValue = hydraConfigGetDisplayResolution().pointee
                self.customDisplayResolution = hydraConfigGetCustomDisplayResolution().pointee
            }
            Spacer()
        }
        Spacer()
    }
}

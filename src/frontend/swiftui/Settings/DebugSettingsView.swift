import SwiftUI

struct DebugSettingsView: View {
    @State var logOutput: HydraLogOutput = HYDRA_LOG_OUTPUT_INVALID
    @State var logFsAccess = false
    @State var debugLogging = false
    // TODO: process args

    var body: some View {
        VStack {
            Picker("Log output", selection: self.$logOutput.rawValue) {
                Text("none (not recommended)").tag(
                    HYDRA_LOG_OUTPUT_NONE.rawValue)
                Text("stdout").tag(
                    HYDRA_LOG_OUTPUT_STD_OUT.rawValue)
                Text("file (default)").tag(HYDRA_LOG_OUTPUT_FILE.rawValue)
            }
            .onChange(of: self.logOutput.rawValue) { _, newValue in
                var logOutputOption = hydraConfigGetLogOutput()
                logOutputOption.value = newValue
            }
            Toggle("Log filesystem access", isOn: self.$logFsAccess)
                .onChange(of: self.logFsAccess) { _, newValue in
                    var logFsAccessOption = hydraConfigGetLogFsAccess()
                    logFsAccessOption.value = newValue
                }
            Toggle("Debug logging", isOn: self.$debugLogging)
                .onChange(of: self.debugLogging) { _, newValue in
                    var debugLoggingOption = hydraConfigGetDebugLogging()
                    debugLoggingOption.value = newValue
                }
            // TODO: process arguments
        }
        .onAppear {
            let logOutputOption = hydraConfigGetLogOutput()
            self.logOutput.rawValue = logOutputOption.value

            let logFsAccessOption = hydraConfigGetLogFsAccess()
            self.logFsAccess = logFsAccessOption.value

            let debugLoggingOption = hydraConfigGetDebugLogging()
            self.debugLogging = debugLoggingOption.value
        }
    }
}

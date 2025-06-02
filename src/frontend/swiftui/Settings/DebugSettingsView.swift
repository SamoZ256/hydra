import SwiftUI

struct DebugSettingsView: View {
    @State var loggingOutput: HydraLoggingOutput = HYDRA_LOGGING_OUTPUT_INVALID
    @State var debugLogging = false
    @State var stackTraceLogging = false
    // TODO: process args

    var body: some View {
        VStack {
            Picker("Logging output", selection: self.$loggingOutput.rawValue) {
                Text("stdout (for debugging)").tag(
                    HYDRA_LOGGING_OUTPUT_STD_OUT.rawValue)
                Text("file").tag(HYDRA_LOGGING_OUTPUT_FILE.rawValue)
            }
            Toggle("Debug logging", isOn: self.$debugLogging)
            Toggle("Stack trace logging", isOn: self.$stackTraceLogging)
        }
        .onAppear {
            load()
        }
        .onDisappear {
            save()
        }
    }

    func load() {
        let loggingOutputOption = hydra_config_get_logging_output()
        self.loggingOutput.rawValue = hydra_u32_option_get(loggingOutputOption)

        let debugLoggingOption = hydra_config_get_debug_logging()
        self.debugLogging = hydra_bool_option_get(debugLoggingOption)

        let stackTraceLoggingOption = hydra_config_get_stack_trace_logging()
        self.stackTraceLogging = hydra_bool_option_get(stackTraceLoggingOption)
    }

    func save() {
        let loggingOutputOption = hydra_config_get_logging_output()
        hydra_u32_option_set(loggingOutputOption, self.loggingOutput.rawValue)

        let debugLoggingOption = hydra_config_get_debug_logging()
        hydra_bool_option_set(debugLoggingOption, self.debugLogging)

        let stackTraceLoggingOption = hydra_config_get_stack_trace_logging()
        hydra_bool_option_set(stackTraceLoggingOption, self.stackTraceLogging)
    }
}

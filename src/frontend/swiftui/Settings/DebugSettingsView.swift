import SwiftUI

struct DebugSettingsView: View {
    @State private var logOutput: HydraLogOutput = HYDRA_LOG_OUTPUT_INVALID
    @State private var logFsAccess = false
    @State private var debugLogging = false
    // TODO: process args
    @State private var gdbEnabled = false
    @State private var gdbPort: UInt16 = 0
    @State private var gdbWaitForClient = false

    var body: some View {
        Spacer()
        HStack {
            Spacer()
            Form {
                Section("Logging") {
                    Picker("Log output", selection: self.$logOutput.rawValue) {
                        Text("none (not recommended)")
                            .tag(HYDRA_LOG_OUTPUT_NONE.rawValue)
                        Text("stdout")
                            .tag(HYDRA_LOG_OUTPUT_STD_OUT.rawValue)
                        Text("file (default)")
                            .tag(HYDRA_LOG_OUTPUT_FILE.rawValue)
                    }
                    .onChange(of: self.logOutput.rawValue) { _, newValue in
                        hydraConfigGetLogOutput().pointee = newValue
                    }

                    Toggle("Log filesystem access", isOn: self.$logFsAccess)
                        .onChange(of: self.logFsAccess) { _, newValue in
                            hydraConfigGetLogFsAccess().pointee = newValue
                        }

                    Toggle("Debug logging", isOn: self.$debugLogging)
                        .onChange(of: self.debugLogging) { _, newValue in
                            hydraConfigGetDebugLogging().pointee = newValue
                        }
                }
                // TODO: process arguments
                Section("GDB") {
                    Toggle("Enabled", isOn: self.$gdbEnabled)
                        .onChange(of: self.gdbEnabled) { _, newValue in
                            hydraConfigGetGdbEnabled().pointee = newValue
                        }
                    TextField("Port", value: self.$gdbPort, formatter: NumberFormatter())
                        .onChange(of: self.gdbPort) { _, newValue in
                            hydraConfigGetGdbPort().pointee = newValue
                        }
                    Toggle("Wait for client", isOn: self.$gdbWaitForClient)
                        .onChange(of: self.gdbWaitForClient) { _, newValue in
                            hydraConfigGetGdbWaitForClient().pointee = newValue
                        }
                }
            }
            .formStyle(.grouped)
            .onAppear {
                self.logOutput.rawValue = hydraConfigGetLogOutput().pointee
                self.logFsAccess = hydraConfigGetLogFsAccess().pointee
                self.debugLogging = hydraConfigGetDebugLogging().pointee
                self.gdbEnabled = hydraConfigGetGdbEnabled().pointee
                self.gdbPort = hydraConfigGetGdbPort().pointee
                self.gdbWaitForClient = hydraConfigGetGdbWaitForClient().pointee
            }
            Spacer()
        }
        Spacer()
    }
}

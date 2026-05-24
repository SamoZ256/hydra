import SwiftUI

#if HYDRA_DEBUG
    let debugLoggingEnabled = true
#else
    let debugLoggingEnabled = false
#endif

struct DebugSettingsView: View {
    @State private var logOutput = HydraLogOutput(rawValue: hydraConfigGetLogOutput().pointee)
    @State private var logFsAccess = hydraConfigGetLogFsAccess().pointee
    @State private var debugLogging = hydraConfigGetDebugLogging().pointee

    // TODO: process args

    @State private var recoverFromSegfault = hydraConfigGetRecoverFromSegfault().pointee

    @State private var gdbEnabled = hydraConfigGetGdbEnabled().pointee
    @State private var gdbPort = hydraConfigGetGdbPort().pointee
    @State private var gdbWaitForClient = hydraConfigGetGdbWaitForClient().pointee

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
                        .disabled(!debugLoggingEnabled)
                        .onChange(of: self.debugLogging) { _, newValue in
                            hydraConfigGetDebugLogging().pointee = newValue
                        }
                }

                // TODO: process arguments

                Section("Error handling") {
                    Toggle("Recover from segfault", isOn: self.$recoverFromSegfault)
                        .onChange(of: self.recoverFromSegfault) { _, newValue in
                            hydraConfigGetRecoverFromSegfault().pointee = newValue
                        }
                }

                Section("GDB") {
                    Toggle("Enabled", isOn: self.$gdbEnabled)
                        .onChange(of: self.gdbEnabled) { _, newValue in
                            hydraConfigGetGdbEnabled().pointee = newValue
                        }
                    if (self.gdbEnabled) {
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
            }
            .formStyle(.grouped)
            .onAppear {
                // TODO: process args
            }
            Spacer()
        }
        Spacer()
    }
}

import SwiftUI

#if HYDRA_DEBUG
    let debugLoggingEnabled = true
#else
    let debugLoggingEnabled = false
#endif

struct DebugSettingsView: View {
    @State private var logOutput = HydraLogOutput(rawValue: configGetLogOutput().pointee)
    @State private var logFsAccess = configGetLogFsAccess().pointee
    @State private var debugLogging = configGetDebugLogging().pointee

    // TODO: process args

    @State private var recoverFromSegfault = configGetRecoverFromSegfault().pointee

    @State private var gdbEnabled = configGetGdbEnabled().pointee
    @State private var gdbPort = configGetGdbPort().pointee
    @State private var gdbWaitForClient = configGetGdbWaitForClient().pointee

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
                        configGetLogOutput().pointee = newValue
                    }

                    Toggle("Log filesystem access", isOn: self.$logFsAccess)
                        .onChange(of: self.logFsAccess) { _, newValue in
                            configGetLogFsAccess().pointee = newValue
                        }

                    Toggle("Debug logging", isOn: self.$debugLogging)
                        .disabled(!debugLoggingEnabled)
                        .onChange(of: self.debugLogging) { _, newValue in
                            configGetDebugLogging().pointee = newValue
                        }
                }

                // TODO: process arguments

                Section("Error handling") {
                    Toggle("Recover from segfault", isOn: self.$recoverFromSegfault)
                        .onChange(of: self.recoverFromSegfault) { _, newValue in
                            configGetRecoverFromSegfault().pointee = newValue
                        }
                }

                Section("GDB") {
                    Toggle("Enabled", isOn: self.$gdbEnabled)
                        .onChange(of: self.gdbEnabled) { _, newValue in
                            configGetGdbEnabled().pointee = newValue
                        }
                    if (self.gdbEnabled) {
                        TextField("Port", value: self.$gdbPort, formatter: NumberFormatter())
                            .onChange(of: self.gdbPort) { _, newValue in
                                configGetGdbPort().pointee = newValue
                            }
                        Toggle("Wait for client", isOn: self.$gdbWaitForClient)
                            .onChange(of: self.gdbWaitForClient) { _, newValue in
                                configGetGdbWaitForClient().pointee = newValue
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

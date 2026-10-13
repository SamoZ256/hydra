import SwiftUI

#if HYDRA_DEBUG
    let debugLoggingEnabled = true
#else
    let debugLoggingEnabled = false
#endif

struct DebugSettingsView: View {
    @State private var logOutput = HydraConfig.global.logOutput
    @State private var logFsAccess = HydraConfig.global.logFsAccess
    @State private var debugLogging = HydraConfig.global.debugLogging

    // TODO: process args

    @State private var recoverFromSegfault = HydraConfig.global.recoverFromSegfault

    @State private var disassemblerBackend = HydraConfig.global.disassemblerBackend

    @State private var gdbEnabled = HydraConfig.global.gdbEnabled
    @State private var gdbPort = HydraConfig.global.gdbPort
    @State private var gdbWaitForClient = HydraConfig.global.gdbWaitForClient

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
                    .onChange(of: self.logOutput) { _, newValue in
                        HydraConfig.global.logOutput = newValue
                    }

                    Toggle("Log filesystem access", isOn: self.$logFsAccess)
                        .onChange(of: self.logFsAccess) { _, newValue in
                            HydraConfig.global.logFsAccess = newValue
                        }

                    Toggle("Debug logging", isOn: self.$debugLogging)
                        .disabled(!debugLoggingEnabled)
                        .onChange(of: self.debugLogging) { _, newValue in
                            HydraConfig.global.debugLogging = newValue
                        }
                }

                // TODO: process arguments

                Section("Error handling") {
                    Toggle("Recover from segfault", isOn: self.$recoverFromSegfault)
                        .onChange(of: self.recoverFromSegfault) { _, newValue in
                            HydraConfig.global.recoverFromSegfault = newValue
                        }
                }

                Section("Disassembling & Decompiling") {
                    Picker("Disassembler backend", selection: self.$disassemblerBackend.rawValue) {
                        Text("none")
                            .tag(HYDRA_DISASSEMBLER_BACKEND_NONE.rawValue)
                        Text("Capstone (default)")
                            .tag(HYDRA_DISASSEMBLER_BACKEND_CAPSTONE.rawValue)
                    }
                    .onChange(of: self.disassemblerBackend) { _, newValue in
                        HydraConfig.global.disassemblerBackend = newValue
                    }
                }

                Section("GDB") {
                    Toggle("Enabled", isOn: self.$gdbEnabled)
                        .onChange(of: self.gdbEnabled) { _, newValue in
                            HydraConfig.global.gdbEnabled = newValue
                        }
                    if self.gdbEnabled {
                        TextField("Port", value: self.$gdbPort, formatter: NumberFormatter())
                            .onChange(of: self.gdbPort) { _, newValue in
                                HydraConfig.global.gdbPort = newValue
                            }
                        Toggle("Wait for client", isOn: self.$gdbWaitForClient)
                            .onChange(of: self.gdbWaitForClient) { _, newValue in
                                HydraConfig.global.gdbWaitForClient = newValue
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

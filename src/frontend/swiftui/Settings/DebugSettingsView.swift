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
                Picker("Log output:", selection: self.$logOutput.rawValue) {
                    Text("none (not recommended)")
                        .tag(HYDRA_LOG_OUTPUT_NONE.rawValue)
                    Text("stdout")
                        .tag(HYDRA_LOG_OUTPUT_STD_OUT.rawValue)
                    Text("file (default)")
                        .tag(HYDRA_LOG_OUTPUT_FILE.rawValue)
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
                Text("GDB")
                Toggle("Enabled", isOn: self.$gdbEnabled)
                    .onChange(of: self.gdbEnabled) { _, newValue in
                        var gdbEnabledOption = hydraConfigGetGdbEnabled()
                        gdbEnabledOption.value = newValue
                    }
                TextField("Port:", value: self.$gdbPort, formatter: NumberFormatter())
                    .onChange(of: self.gdbPort) { _, newValue in
                        var gdbPortOption = hydraConfigGetGdbPort()
                        gdbPortOption.value = newValue
                    }
                Toggle("Wait for client", isOn: self.$gdbWaitForClient)
                    .onChange(of: self.gdbWaitForClient) { _, newValue in
                        var gdbWaitForClientOption = hydraConfigGetGdbWaitForClient()
                        gdbWaitForClientOption.value = newValue
                    }
            }
            .padding(24)
            .padding(.horizontal, 64)
            .onAppear {
                let logOutputOption = hydraConfigGetLogOutput()
                self.logOutput.rawValue = logOutputOption.value
    
                let logFsAccessOption = hydraConfigGetLogFsAccess()
                self.logFsAccess = logFsAccessOption.value
    
                let debugLoggingOption = hydraConfigGetDebugLogging()
                self.debugLogging = debugLoggingOption.value
    
                let gdbEnabledOption = hydraConfigGetGdbEnabled()
                self.gdbEnabled = gdbEnabledOption.value
    
                let gdbPortOption = hydraConfigGetGdbPort()
                self.gdbPort = gdbPortOption.value
    
                let gdbWaitForClientOption = hydraConfigGetGdbWaitForClient()
                self.gdbWaitForClient = gdbWaitForClientOption.value
            }
            Spacer()
        }
        Spacer()
    }
}

import SwiftUI

struct SystemSettingsView: View {
    #if os(macOS)
        @State private var firmwarePath: String = ""
        @State private var sdCardPath: String = ""
        @State private var savePath: String = ""
        @State private var sysmodulesPath: String = ""
    #else
        @State private var handheldMode = true
    #endif

    var body: some View {
        Spacer()
        HStack {
            Spacer()
            Form {
                #if os(macOS)
                    Section("Paths") {
                        // TODO: use file importers
                        TextField("Firmware Path", text: $firmwarePath)
                            .onChange(of: firmwarePath) { _, newValue in
                                hydraConfigSetFirmwarePath(newValue)
                            }
                        TextField("SD Card Path", text: $sdCardPath)
                            .onChange(of: sdCardPath) { _, newValue in
                                hydraConfigSetSdCardPath(newValue)
                            }
                        TextField("Save Path", text: $savePath)
                            .onChange(of: savePath) { _, newValue in
                                hydraConfigSetSavePath(newValue)
                            }
                        TextField("Sysmodules Path", text: $sysmodulesPath)
                            .onChange(of: sysmodulesPath) { _, newValue in
                                hydraConfigSetSysmodulesPath(newValue)
                            }
                    }
                #else
                    Section {
                        Toggle("Handheld mode", isOn: self.$handheldMode)
                            .onChange(of: self.handheldMode) { _, newValue in
                                hydraConfigGetHandheldMode().pointee = newValue
                            }
                    }
                #endif
            }
            .formStyle(.grouped)
            .onAppear {
                #if os(macOS)
                    self.firmwarePath = hydraConfigGetFirmwarePath()
                    self.sdCardPath = hydraConfigGetSdCardPath()
                    self.savePath = hydraConfigGetSavePath()
                    self.sysmodulesPath = hydraConfigGetSysmodulesPath()
                #else
                    self.handheldMode = hydraConfigGetHandheldMode().pointee
                #endif
            }
            Spacer()
        }
        Spacer()
    }
}

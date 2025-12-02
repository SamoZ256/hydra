import SwiftUI

struct SystemSettingsView: View {
    @State private var firmwarePath: String = ""
    @State private var sdCardPath: String = ""
    @State private var savePath: String = ""
    @State private var sysmodulesPath: String = ""

    var body: some View {
        Spacer()
        HStack {
            Spacer()
            Form {
                // TODO: use file importers
                TextField("Firmware Path:", text: $firmwarePath)
                    .onChange(of: firmwarePath) { _, newValue in
                        var firmwarePathOption = hydraConfigGetFirmwarePath()
                        firmwarePathOption.value = newValue
                    }
                TextField("SD Card Path:", text: $sdCardPath)
                    .onChange(of: sdCardPath) { _, newValue in
                        var sdCardPathOption = hydraConfigGetSdCardPath()
                        sdCardPathOption.value = newValue
                    }
                TextField("Save Path:", text: $savePath)
                    .onChange(of: savePath) { _, newValue in
                        var savePathOption = hydraConfigGetSavePath()
                        savePathOption.value = newValue
                    }
                TextField("Sysmodules Path:", text: $sysmodulesPath)
                    .onChange(of: sysmodulesPath) { _, newValue in
                        var sysmodulesPathOption = hydraConfigGetSysmodulesPath()
                        sysmodulesPathOption.value = newValue
                    }
            }
            .padding(24)
            .padding(.horizontal, 24)
            .onAppear {
                let firmwarePathOption = hydraConfigGetFirmwarePath()
                self.firmwarePath = firmwarePathOption.value
    
                let sdCardPathOption = hydraConfigGetSdCardPath()
                self.sdCardPath = sdCardPathOption.value
    
                let savePathOption = hydraConfigGetSavePath()
                self.savePath = savePathOption.value
    
                let sysmodulesPathOption = hydraConfigGetSysmodulesPath()
                self.sysmodulesPath = sysmodulesPathOption.value
            }
            Spacer()
        }
        Spacer()
    }
}

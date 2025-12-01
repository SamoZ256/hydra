import SwiftUI

struct SystemSettingsView: View {
    @State var firmwarePath: String = ""
    @State var sdCardPath: String = ""
    @State var savePath: String = ""
    @State var sysmodulesPath: String = ""

    var body: some View {
        GeometryReader { geo in
            GroupBox {
                VStack {
                    // TODO: use file importers
                    HStack {
                        Text("Firmware Path:")
                        TextField("Firmware Path", text: $firmwarePath)
                            .onChange(of: firmwarePath) { _, newValue in
                                var firmwarePathOption = hydraConfigGetFirmwarePath()
                                firmwarePathOption.value = newValue
                            }
                    }
                    HStack {
                        Text("SD Card Path:")
                        TextField("SD Card Path", text: $sdCardPath)
                            .onChange(of: sdCardPath) { _, newValue in
                                var sdCardPathOption = hydraConfigGetSdCardPath()
                                sdCardPathOption.value = newValue
                            }
                    }
                    HStack {
                        Text("Save Path:")
                        TextField("Save Path", text: $savePath)
                            .onChange(of: savePath) { _, newValue in
                                var savePathOption = hydraConfigGetSavePath()
                                savePathOption.value = newValue
                            }
                    }
                    HStack {
                        Text("Sysmodules Path:")
                        TextField("Sysmodules Path", text: $sysmodulesPath)
                            .onChange(of: sysmodulesPath) { _, newValue in
                                var sysmodulesPathOption = hydraConfigGetSysmodulesPath()
                                sysmodulesPathOption.value = newValue
                            }
                    }
                }
                .frame(width: geo.size.width, height: geo.size.height)
            }
        }
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
    }
}

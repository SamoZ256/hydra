import SwiftUI

struct SystemSettingsView: View {
    @State private var deviceNickname = HydraConfig.global.deviceNickname
    @State private var systemLanguage = HydraConfig.global.systemLanguage
    @State private var systemLocation = HydraConfig.global.systemLocation
    #if os(macOS)
        @State private var firmwarePath = HydraConfig.global.firmwarePath
        @State private var sdCardPath = HydraConfig.global.sdCardPath
        @State private var savePath = HydraConfig.global.savePath
        @State private var sysmodulesPath = HydraConfig.global.sysmodulesPath
    #else
        @State private var handheldMode = HydraConfig.global.handheldMode
    #endif

    @State private var systemLocations: [String] = []

    var body: some View {
        Spacer()
        HStack {
            Spacer()
            Form {
                TextField("Device nickname", text: $deviceNickname)
                    .onChange(of: deviceNickname) { _, newValue in
                        HydraConfig.global.deviceNickname = newValue
                    }

                Picker("System language", selection: self.$systemLanguage.rawValue) {
                    Text("American English").tag(HYDRA_SYSTEM_LANGUAGE_AMERICAN_ENGLISH.rawValue)
                    Text("British English").tag(HYDRA_SYSTEM_LANGUAGE_BRITISH_ENGLISH.rawValue)
                    Text("Japanese").tag(HYDRA_SYSTEM_LANGUAGE_JAPANESE.rawValue)
                    Text("French").tag(HYDRA_SYSTEM_LANGUAGE_FRENCH.rawValue)
                    Text("German").tag(HYDRA_SYSTEM_LANGUAGE_GERMAN.rawValue)
                    Text("Latin American Spanish").tag(
                        HYDRA_SYSTEM_LANGUAGE_LATIN_AMERICAN_SPANISH.rawValue)
                    Text("Spanish").tag(HYDRA_SYSTEM_LANGUAGE_SPANISH.rawValue)
                    Text("Italian").tag(HYDRA_SYSTEM_LANGUAGE_ITALIAN.rawValue)
                    Text("Dutch").tag(HYDRA_SYSTEM_LANGUAGE_DUTCH.rawValue)
                    Text("Canadian French").tag(HYDRA_SYSTEM_LANGUAGE_CANADIAN_FRENCH.rawValue)
                    Text("Portuguese").tag(HYDRA_SYSTEM_LANGUAGE_PORUGUESE.rawValue)
                    Text("Russian").tag(HYDRA_SYSTEM_LANGUAGE_RUSSIAN.rawValue)
                    Text("Korean").tag(HYDRA_SYSTEM_LANGUAGE_KOREAN.rawValue)
                    Text("Traditional Chinese").tag(
                        HYDRA_SYSTEM_LANGUAGE_TRADITIONAL_CHINESE.rawValue)
                    Text("Simplified Chinese").tag(
                        HYDRA_SYSTEM_LANGUAGE_SIMPLIFIED_CHINESE.rawValue)
                    Text("Brazilian Portuguese").tag(
                        HYDRA_SYSTEM_LANGUAGE_BRAZILIAN_PORTUGUESE.rawValue)
                    Text("Polish").tag(HYDRA_SYSTEM_LANGUAGE_POLISH.rawValue)
                    Text("Thai").tag(HYDRA_SYSTEM_LANGUAGE_THAI.rawValue)
                }
                .onChange(of: self.systemLanguage) { _, newValue in
                    HydraConfig.global.systemLanguage = newValue
                    // TODO: reload titles and icons
                }

                Picker("System location", selection: self.$systemLocation) {
                    Text("auto").tag("auto")

                    Divider()

                    ForEach(self.systemLocations, id: \.self) { location in
                        Text(location)
                    }
                }
                .onChange(of: self.systemLocation) { _, newValue in
                    HydraConfig.global.systemLocation = newValue
                }
                .onAppear {
                    let filesystem = HydraFilesystem()
                    let timeZoneManager = HydraTimeZoneManager(filesystem: filesystem)
                    self.systemLocations.reserveCapacity(timeZoneManager.locationCount)
                    for i in 0..<timeZoneManager.locationCount {
                        self.systemLocations.append(timeZoneManager.getLocation(at: i))
                    }
                }

                #if os(macOS)
                    Section("Paths") {
                        // TODO: use file importers
                        TextField("Firmware Path", text: $firmwarePath)
                            .onChange(of: firmwarePath) { _, newValue in
                                HydraConfig.global.firmwarePath = newValue
                            }
                        TextField("SD Card Path", text: $sdCardPath)
                            .onChange(of: sdCardPath) { _, newValue in
                                HydraConfig.global.sdCardPath = newValue
                            }
                        TextField("Save Path", text: $savePath)
                            .onChange(of: savePath) { _, newValue in
                                HydraConfig.global.savePath = newValue
                            }
                        TextField("Sysmodules Path", text: $sysmodulesPath)
                            .onChange(of: sysmodulesPath) { _, newValue in
                                HydraConfig.global.sysmodulesPath = newValue
                            }
                    }
                #else
                    Section {
                        Toggle("Handheld mode", isOn: self.$handheldMode)
                            .onChange(of: self.handheldMode) { _, newValue in
                                HydraConfig.global.handheldMode = newValue
                            }
                    }
                #endif
            }
            .formStyle(.grouped)
            Spacer()
        }
        Spacer()
    }
}

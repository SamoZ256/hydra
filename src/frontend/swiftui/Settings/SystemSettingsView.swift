import SwiftUI

struct SystemSettingsView: View {
    @State private var systemLanguage = HydraSystemLanguage(rawValue: hydraConfigGetSystemLanguage().pointee)
    @State private var systemLocation = hydraConfigGetSystemLocation()
    #if os(macOS)
        @State private var firmwarePath = hydraConfigGetFirmwarePath()
        @State private var sdCardPath = hydraConfigGetSdCardPath()
        @State private var savePath = hydraConfigGetSavePath()
        @State private var sysmodulesPath = hydraConfigGetSysmodulesPath()
    #else
        @State private var handheldMode = hydraConfigGetHandheldMode().pointee
    #endif

    @State private var systemLocations: [String] = []

    var body: some View {
        Spacer()
        HStack {
            Spacer()
            Form {
                Picker("System language", selection: self.$systemLanguage.rawValue) {
                    Text("American English").tag(HYDRA_SYSTEM_LANGUAGE_AMERICAN_ENGLISH.rawValue)
                    Text("British English").tag(HYDRA_SYSTEM_LANGUAGE_BRITISH_ENGLISH.rawValue)
                    Text("Japanese").tag(HYDRA_SYSTEM_LANGUAGE_JAPANESE.rawValue)
                    Text("French").tag(HYDRA_SYSTEM_LANGUAGE_FRENCH.rawValue)
                    Text("German").tag(HYDRA_SYSTEM_LANGUAGE_GERMAN.rawValue)
                    Text("Latin American Spanish").tag(HYDRA_SYSTEM_LANGUAGE_LATIN_AMERICAN_SPANISH.rawValue)
                    Text("Spanish").tag(HYDRA_SYSTEM_LANGUAGE_SPANISH.rawValue)
                    Text("Italian").tag(HYDRA_SYSTEM_LANGUAGE_ITALIAN.rawValue)
                    Text("Dutch").tag(HYDRA_SYSTEM_LANGUAGE_DUTCH.rawValue)
                    Text("Canadian French").tag(HYDRA_SYSTEM_LANGUAGE_CANADIAN_FRENCH.rawValue)
                    Text("Portuguese").tag(HYDRA_SYSTEM_LANGUAGE_PORUGUESE.rawValue)
                    Text("Russian").tag(HYDRA_SYSTEM_LANGUAGE_RUSSIAN.rawValue)
                    Text("Korean").tag(HYDRA_SYSTEM_LANGUAGE_KOREAN.rawValue)
                    Text("Traditional Chinese").tag(HYDRA_SYSTEM_LANGUAGE_TRADITIONAL_CHINESE.rawValue)
                    Text("Simplified Chinese").tag(HYDRA_SYSTEM_LANGUAGE_SIMPLIFIED_CHINESE.rawValue)
                    Text("Brazilian Portuguese").tag(HYDRA_SYSTEM_LANGUAGE_BRAZILIAN_PORTUGUESE.rawValue)
                    Text("Polish").tag(HYDRA_SYSTEM_LANGUAGE_POLISH.rawValue)
                    Text("Thai").tag(HYDRA_SYSTEM_LANGUAGE_THAI.rawValue)
                }
                .onChange(of: self.systemLanguage.rawValue) { _, newValue in
                    hydraConfigGetSystemLanguage().pointee = newValue
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
                    hydraConfigSetSystemLocation(newValue)
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
            Spacer()
        }
        Spacer()
    }
}

import SwiftUI
import UniformTypeIdentifiers

private let switchType = UTType(exportedAs: "com.samoz256.switch-document", conformingTo: .data)
#if os(macOS)
    private let allowedContentTypes: [UTType] = [.folder, switchType]

    private let installFirmwarePlacement = ToolbarItemPlacement.confirmationAction
    private let addGamePlacement = ToolbarItemPlacement.confirmationAction
#else
    private let allowedContentTypes: [UTType] = [switchType]  // No game folders on iOS

    private let installFirmwarePlacement = ToolbarItemPlacement.topBarLeading
    private let addGamePlacement = ToolbarItemPlacement.topBarTrailing
#endif

struct GameListToolbarItems: ToolbarContent {
    @Binding var activeGame: Game?
    @Binding var emulationContext: HydraEmulationContext?

    @State private var isFirmwareFilePickerPresented = false
    @State private var isGameFilePickerPresented = false
    @State private var showingFirmwareImportError = false

    var body: some ToolbarContent {
        var firmwarePathOption = hydraConfigGetFirmwarePath()
        if firmwarePathOption.value.isEmpty {
            ToolbarItem(placement: installFirmwarePlacement) {
                Button("Install Firmware") {
                    isFirmwareFilePickerPresented = true
                }
                .fileImporter(
                    isPresented: $isFirmwareFilePickerPresented,
                    allowedContentTypes: [.folder],
                    allowsMultipleSelection: false
                ) { result in
                    switch result {
                    case .success(let fileURLs):
                        for fileURL in fileURLs {
                            guard fileURL.startAccessingSecurityScopedResource() else {
                                showingFirmwareImportError = true
                                return
                            }

                            defer { fileURL.stopAccessingSecurityScopedResource() }

                            #if os(macOS)
                                firmwarePathOption.value = fileURL.path
                            #else
                                let path = "\(hydraConfigGetAppDataPath())/firmware"
                                do {
                                    try FileManager.default.copyItem(
                                        atPath: fileURL.path, toPath: firmwarePathOption.value)
                                } catch {
                                    showingFirmwareImportError = true
                                }

                                firmwarePathOption.value = path
                            #endif

                            hydraConfigSerialize()
                        }
                    case .failure(let error):
                        print(error)
                    }
                }
                .alert("Failed to import firmware", isPresented: $showingFirmwareImportError) {
                    Button("OK", role: .cancel) {}
                }
            }
        }

        #if os(macOS)
            ToolbarItemGroup(placement: .principal) {
                Button("List View", systemImage: "list.bullet") {}
                Button("Grid View", systemImage: "rectangle.grid.3x2.fill") {}
            }
        #endif

        ToolbarItem(placement: addGamePlacement) {
            Button("Add Game", systemImage: "plus") {
                isGameFilePickerPresented = true
            }
            .fileImporter(
                isPresented: $isGameFilePickerPresented,
                allowedContentTypes: allowedContentTypes,
                allowsMultipleSelection: true
            ) { result in
                switch result {
                case .success(let fileURLs):
                    for fileURL in fileURLs {
                        do {
                            try registerUrl(fileURL)
                        } catch {
                            print("Failed to register URL \(fileURL.path)")
                            continue
                        }

                        let gamePathsOption = hydraConfigGetGamePaths()
                        gamePathsOption.append(
                            value: fileURL.path)

                        hydraConfigSerialize()
                    }
                case .failure(let error):
                    print(error)
                }
            }
        }

        #if os(iOS)
            ToolbarItem(placement: .topBarTrailing) {
                NavigationLink(destination: SettingsView()) {
                    Image(systemName: "gear")
                }
            }
        #endif
    }

}
